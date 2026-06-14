# Trickster Rich Presence Client

## ✨ Features

- Displays player status (**Login Screen / In-Game**)
- Shows:
  - Character Name  
  - Character Job  
  - Character Level  
  - Current Map  
- Includes:
  - A **general area map image**
  - A **character job image**
- Designed primarily for **S2 game clients**

---

## ⚙ How It Works

- This project works by **reading values directly from the game client memory** where the information is stored.
- **No data is required from the game server**.
- When using multiple game clients at the same time, **only the first detected client will update Discord Rich Presence**.

---

## 🔄 Discord Handling

- The program safely handles Discord being:
  - Closed and reopened during gameplay
  - Opened after the game has already started
- Once Discord is detected, the Rich Presence status is automatically updated.

---

## ⚠ Compatibility Notes

- This project may require modifications to work with your own private server client (for example, memory pointers may differ).
- You must create your own **DLL injector** to inject this library into the game client.
  - The recommended approach is to use a **custom game launcher** for injection.

---

## 📝 Customization

For personal use, it is recommended to rename the project and files to something other than:

`PTRPC (Prifma Trickster Rich Presence Client)`

This helps avoid confusion and keeps your implementation unique.

---

## 🖥 Architecture

- **32-bit only** by default  
  - Can be adapted for **64-bit** with changes
- On my server:
  - The launcher is **32-bit** for retro-compatibility  
  - The DLL is also **32-bit**

---

## 📄 License & Usage

This project is provided **free of charge** and **as-is**.  
See the examples for intended use cases.

- ❌ No donations accepted  
- ❌ No resale permitted  
- ✅ Free for learning and personal server projects  

Please note : i am not an expert developper, cleaning the code, optimisation and checks may be needed, random garbage may have been left behind. I also have the bad habbit of barely adding comments to my code. 

## Disclaimer

This project is free and publicly provided by me.

I have been made aware that someone copied my work and resold it without permission. I do not authorize anyone to sell this code, claim it as their own, or resell “modified” versions of it as original work.

If you paid for this code, or someone claimed they created it, you may have been scammed. The official version is available here for free.

To know if your server’s Rich Presence is a fork or copy of mine, check the asset images. If the asset images are the same as the ones used in this project, then it was taken from my work.

If this was provided to you for free, then all is good. My issue is not with people using it — it is with people taking free work, claiming ownership, and reselling it without permission.
