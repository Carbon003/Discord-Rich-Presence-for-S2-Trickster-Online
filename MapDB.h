#pragma once
#include <string>

bool GetMapName(int id, std::string& outName);                 // already there
bool GetMapInfo(int id, std::string& outAsset, std::string& outTip); // ← NEW
