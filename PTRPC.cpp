// PTRPC.cpp  –  Prifma Trickster Online Rich-Presence injector

#include "pch.h"
#include <windows.h>
#include <tlhelp32.h>
#include <Psapi.h>
#include <fstream>
#include <thread>
#include <chrono>
#include <string>
#include <climits>   // for INT_MIN
#include "discord.h"
#include "MapDB.h"
#include "ClassDB.h"
#pragma comment(lib, "discord_game_sdk.lib")
#pragma comment(lib, "Psapi.lib")

// -------- Feature toggle --------
static bool DisableClass = false;   // set true to ship without class

// -------- Logging --------
static void Log(const char* msg) {
    std::ofstream o("C:\\temp\\PTRPC_log.txt", std::ios::app);
    if (o.is_open()) o << msg << '\n';
}
static void Log(const std::string& s) { Log(s.c_str()); }

// -------- Process helper --------
static bool IsProcessRunning(const wchar_t* exeName)
{
    PROCESSENTRY32W pe{ sizeof(pe) };
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return false;
    if (!Process32FirstW(snap, &pe)) { CloseHandle(snap); return false; }
    do {
        if (wcscmp(pe.szExeFile, exeName) == 0) {
            CloseHandle(snap);
            return true;
        }
    } while (Process32NextW(snap, &pe));
    CloseHandle(snap);
    return false;
}

// -------- stable globals --------
constexpr uintptr_t NAME_ADDR = 0x00DAFA0C;  // char[64]
constexpr uintptr_t LEVEL_ADDR = 0x00DB7924;  // int32
constexpr uintptr_t MAP_ID_ADDR = 0x00CDECFC;  // int32

// -------- SAFE memory reads (only place we use __try) --------
extern "C" __declspec(noinline)
uintptr_t SafeReadPtr(uintptr_t addr) {
    __try { return *reinterpret_cast<uintptr_t*>(addr); }
    __except (EXCEPTION_EXECUTE_HANDLER) { return 0; }
}

extern "C" __declspec(noinline)
bool SafeReadInt32(uintptr_t addr, int& out) {
    __try { out = *reinterpret_cast<volatile int*>(addr); return true; }
    __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
}

// -------- pointer-chain resolver (no __try here) --------
static inline uintptr_t MainModuleBase() {
    return reinterpret_cast<uintptr_t>(GetModuleHandleW(nullptr)); // main EXE
}

struct Chain {
    uint32_t       baseOff;  // module base + baseOff, then deref
    const uint32_t* offs;    // offsets array (derefs for all but last)
    size_t         len;      // length of offs
    const char* label;    // for logs
};

// Offsets from your pointer-scan screenshot
static const uint32_t CH1_OFFS[] = { 0xB8, 0x10, 0x00, 0x04, 0x28, 0x04, 0x3B0 };
static const uint32_t CH2_OFFS[] = { 0xD0, 0x10, 0x00, 0x04, 0x28, 0x04, 0x3B0 };
static const uint32_t CH3_OFFS[] = { 0x1BC,0x10, 0x00, 0x04, 0x28, 0x04, 0x3B0 };
static const uint32_t CH4_OFFS[] = { 0xC4, 0x10, 0x00, 0x04, 0x28, 0x04, 0x3B0 };
static const uint32_t CH5_OFFS[] = { 0xDC, 0x10, 0x00, 0x04, 0x28, 0x04, 0x3B0 };
static const uint32_t CH6_OFFS[] = { 0xE4, 0x10, 0x00, 0x04, 0x28, 0x04, 0x3B0 };
static const uint32_t CH7_OFFS[] = { 0xFC, 0x10, 0x00, 0x04, 0x28, 0x04, 0x3B0 };

static const Chain kChains[] = {
    { 0x003BAC0,   CH1_OFFS, _countof(CH1_OFFS), "3BAC0->B8,10,0,4,28,4,3B0" },
    { 0x003BEFB,   CH2_OFFS, _countof(CH2_OFFS), "3BEFB->D0,10,0,4,28,4,3B0" },
    { 0x003157C4,  CH3_OFFS, _countof(CH3_OFFS), "3157C4->1BC,10,0,4,28,4,3B0" },
    { 0x0030BAC0,  CH4_OFFS, _countof(CH4_OFFS), "30BAC0->C4,10,0,4,28,4,3B0" },
    { 0x0030BEF8,  CH5_OFFS, _countof(CH5_OFFS), "30BEF8->DC,10,0,4,28,4,3B0" },
    { 0x0030BAC0,  CH6_OFFS, _countof(CH6_OFFS), "30BAC0->E4,10,0,4,28,4,3B0" },
    { 0x0030BEF8,  CH7_OFFS, _countof(CH7_OFFS), "30BEF8->FC,10,0,4,28,4,3B0" },
};

static uintptr_t ResolveLeafAddr(const Chain& c)
{
    uintptr_t mod = MainModuleBase();
    if (!mod) return 0;

    uintptr_t p = SafeReadPtr(mod + c.baseOff);
    if (!p || c.len == 0) return 0;

    // walk: deref for all but last
    for (size_t i = 0; i + 1 < c.len; ++i) {
        p = SafeReadPtr(p + c.offs[i]);
        if (!p) return 0;
    }
    // leaf
    return p + c.offs[c.len - 1];
}

static bool ReadClassId_ResolvingOnce(int& outCid)
{
    static uintptr_t s_leafAddr = 0; // cached address of the int

    if (!s_leafAddr) {
        for (const auto& ch : kChains) {
            uintptr_t addr = ResolveLeafAddr(ch);
            if (!addr) continue;

            int probe = -1;
            if (SafeReadInt32(addr, probe) && probe >= 0 && probe <= 5000) {
                s_leafAddr = addr;
                char b[160];
                sprintf_s(b, "[PTRPC] class chain resolved: %s -> addr=%p val=%d",
                    ch.label, (void*)addr, probe);
                Log(b);
                break;
            }
        }
        if (!s_leafAddr) return false;
    }

    int v = 0;
    if (!SafeReadInt32(s_leafAddr, v)) {
        // pointer died (map change, etc.) — clear and retry next tick
        s_leafAddr = 0;
        return false;
    }
    if (v < 0 || v > 5000) return false; // adjust if needed
    outCid = v;
    return true;
}

// -------- DLL boilerplate --------
extern "C" __declspec(dllexport) void __stdcall StartPresence();

static inline int64_t NowUnixSecs() {
    return std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
}

BOOL APIENTRY DllMain(HMODULE, DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH)
        std::thread(StartPresence).detach();
    return TRUE;
}

// -------- main loop --------
extern "C" __declspec(dllexport) void __stdcall StartPresence()
{
    Log("PTRPC injected – worker thread started");

    const uint64_t APP_ID = 953796929555419206ULL;
    discord::Core* core = nullptr;

    auto createCore = [&]() {
        auto r = discord::Core::Create(APP_ID, DiscordCreateFlags_NoRequireDiscord, &core);
        if (r == discord::Result::Ok && core) Log("Discord Core created / reconnected");
        else { char buf[96]; sprintf_s(buf, "Core::Create failed: %d", int(r)); Log(buf); core = nullptr; }
        };
    createCore();

    const int64_t bootSecs = NowUnixSecs();

    // --- per-area timer state (NEW) ---
    int64_t areaStartSecs = bootSecs;
    int      lastAreaKey = INT_MIN;   // -1 = login, else mapId

    std::string lastDetails, lastState, lastClassAsset, lastMapAsset;

    while (true)
    {
        const bool discordUp = IsProcessRunning(L"Discord.exe");

        if (!discordUp && core) { delete core; core = nullptr; }
        if (discordUp && !core) {
            createCore();
            lastDetails.clear(); lastState.clear();
            lastClassAsset.clear(); lastMapAsset.clear();
            // keep areaStartSecs; it will refresh below if area changed while Discord was down
        }

        // ---- read game memory ----
        char nameBuf[64] = {};
        if (!IsBadReadPtr((void*)NAME_ADDR, 1))
            strncpy_s(nameBuf, (const char*)NAME_ADDR, _TRUNCATE);

        int level = 0;
        if (!IsBadReadPtr((void*)LEVEL_ADDR, sizeof(int)))
            level = *reinterpret_cast<int*>(LEVEL_ADDR);

        int mapId = 0;
        if (!IsBadReadPtr((void*)MAP_ID_ADDR, sizeof(int)))
            mapId = *reinterpret_cast<int*>(MAP_ID_ADDR);

        // class via chain
        int  classId = 0;
        bool haveClassId = false;
        if (!DisableClass) {
            haveClassId = ReadClassId_ResolvingOnce(classId);
            char b[128]; sprintf_s(b, "[PTRPC] class read: ok=%d val=%d", int(haveClassId), classId); Log(b);
        }

        // ---- lookup map / class info ----
        std::string mapName;
        if (!GetMapName(mapId, mapName)) mapName = "Unknown Area";

        std::string mapAsset, mapTip;
        if (!GetMapInfo(mapId, mapAsset, mapTip)) { mapAsset = "unknown"; mapTip = "Unknown Area"; }

        std::string classAsset, classDisplay;
        bool haveClass = false;
        if (!DisableClass && haveClassId) {
            haveClass = GetJobInfo(classId, classAsset, classDisplay);
            char b2[200]; sprintf_s(b2, "[PTRPC] GetJobInfo(%d)->%d asset='%s'", classId, int(haveClass), classAsset.c_str()); Log(b2);
        }

        // ---- build presence ----
        bool onLogin = (nameBuf[0] == 0) || (strcmp(nameBuf, "_BASIC_") == 0) || (level <= 1);

        std::string details, state;
        if (onLogin) {
            details = "Prifma Trickster Online – Login Screen";
            state = "Server Select";
            mapAsset = "login"; mapTip = "Login Screen";
        }
        else {
            if (!DisableClass && haveClass)
                details = std::string(nameBuf) + " – " + classDisplay + " – Lv." + std::to_string(level);
            else if (!DisableClass && haveClassId)
                details = std::string(nameBuf) + " – Class#" + std::to_string(classId) + " – Lv." + std::to_string(level);
            else
                details = std::string(nameBuf) + " – Lv." + std::to_string(level);
            state = mapName;
        }

        // ---- area timer reset (NEW) ----
        // Represent the "area": -1 for login screen, otherwise the actual mapId
        int areaKey = onLogin ? -1 : mapId;
        if (areaKey != lastAreaKey) {
            lastAreaKey = areaKey;
            areaStartSecs = NowUnixSecs();
            char b[160];
            sprintf_s(b, "[PTRPC] Area changed → key=%d (mapId=%d) – resetting timer", areaKey, mapId);
            Log(b);
        }

        const bool changed = (details != lastDetails) ||
            (state != lastState) ||
            (!DisableClass && haveClass && (classAsset != lastClassAsset)) ||
            (mapAsset != lastMapAsset);

        // ---- push to Discord ----
        if (discordUp && core && changed)
        {
            discord::Activity act{};
            act.SetDetails(details.c_str());
            act.SetState(state.c_str());

            // was: act.GetTimestamps().SetStart(bootSecs);
            act.GetTimestamps().SetStart(areaStartSecs);     // <-- per-area timer

            act.GetAssets().SetLargeImage(mapAsset.c_str());
            act.GetAssets().SetLargeText(mapTip.c_str());

            if (!DisableClass && haveClass && !onLogin) {
                act.GetAssets().SetSmallImage(classAsset.c_str());
                act.GetAssets().SetSmallText(classDisplay.c_str());
            }

            core->ActivityManager().UpdateActivity(
                act,
                [details, state, mapAsset](discord::Result r) {
                    char buf[256];
                    sprintf_s(buf, "UpdateActivity → %s | %s | img=%s  (%d)",
                        details.c_str(), state.c_str(), mapAsset.c_str(), int(r));
                    Log(buf);
                });

            lastDetails = details;
            lastState = state;
            if (!DisableClass && haveClass) lastClassAsset = classAsset;
            lastMapAsset = mapAsset;
        }

        if (discordUp && core) {
            if (core->RunCallbacks() != discord::Result::Ok) {
                Log("RunCallbacks error – resetting Core");
                delete core; core = nullptr;
            }
        }

        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}
