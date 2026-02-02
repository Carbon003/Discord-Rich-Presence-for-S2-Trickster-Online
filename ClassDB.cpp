// ClassDB.cpp  –  job-ID → { asset key, display text } lookup
#include "ClassDB.h"
#include <unordered_map>

// convenience alias
using Entry = std::pair<const char*, const char*>;     // {asset, display}

static const std::unordered_map<int, Entry> g_Table = {
    {  1, {"schoolgirl",   "Schoolgirl"}      },
    {  2, {"fighter",      "Fighter"}         },
    {  3, {"librarian",    "Librarian"}       },
    {  4, {"shaman",       "Shaman"}          },
    {  5, {"archeologist", "Archeologist"}    },
    {  6, {"engineer",     "Engineer"}        },
    {  7, {"model",        "Model"}           },
    {  8, {"teacher",      "Teacher"}         },
    {  9, {"boxer",        "Boxer"}           },
    { 10, {"warrior",      "Warrior"}         },
    { 11, {"bard",         "Bard"}            },
    { 12, {"magician",     "Magician"}        },
    { 13, {"explorer",     "Explorer"}        },
    { 14, {"inventor",     "Inventor"}        },
    { 15, {"entertainer",  "Entertainer"}     },
    { 16, {"card_master",  "Card Master"}     },
    { 17, {"champion",     "Champion"}        },
    { 18, {"duelist",      "Duelist"}         },
    { 19, {"mercenary",    "Mercenary"}       },
    { 20, {"gladiator",    "Gladiator"}       },
    { 21, {"soul_master",  "Soul Master"}     },
    { 22, {"witch",        "Witch"}           },
    { 23, {"wizard",       "Wizard"}          },
    { 24, {"darklord",     "Dark Lord"}       },
    { 25, {"priest",       "Priest"}          },
    { 26, {"thief_master", "Thief Master"}    },
    { 27, {"hunter_lord",  "Hunter Lord"}     },
    { 28, {"cyber_hunter", "Cyber Hunter"}    },
    { 29, {"scientist",    "Scientist"}       },
    { 30, {"primadonna",   "Primadonna"}      },
    { 31, {"diva",         "Diva"}            },
    { 32, {"duke",         "Duke"}            },
    { 33, {"gambler",      "Gambler"}         }
};

bool GetJobInfo(int id,
    std::string& asset,
    std::string& display)
{
    auto it = g_Table.find(id);
    if (it == g_Table.end()) return false;
    asset = it->second.first;
    display = it->second.second;
    return true;
}
