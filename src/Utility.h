#ifndef UTILITY_H
#define UTILITY_H

#include <SimpleIni.h>
#include <spdlog/sinks/basic_file_sink.h>

#include <fstream>
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include "StringSetting.h"

namespace logger = SKSE::log;

extern int menuHotkey;
extern int widgetX;
extern int widgetY;
extern int enableWidget;
extern bool hintShown;

struct VariantInfo {
    std::string creature;
    std::string category;
    std::string localizedName;
};

struct CreatureData {
    int kills = 0;
    int summons = 0;
    int transformations = 0;

    void Serialize(SKSE::SerializationInterface* intfc) const;
    bool Deserialize(SKSE::SerializationInterface* intfc);
};

extern std::unordered_map<std::string, VariantInfo> VariantMap;
extern std::unordered_set<std::string> BestiaryUnlockedCreatures;
extern std::unordered_map<std::string, CreatureData> BestiaryDataMap;

inline static RE::StringSetting bestiaryTutorialMessage = {"sBestiaryTutorialMessage", "Press [{}] to access your bestiary."};

static void Install() { 
       RE::GameSettingCollection* collection = RE::GameSettingCollection::GetSingleton();
    collection->InsertSetting(bestiaryTutorialMessage);
}

void SetupLog();
void LoadDataFromINI();
void PopulateVariantMap();
std::string GetVariantName(const std::string& filePath);
void ShowTutorialHintText();
void HideTutorialHintText();
void CheckAndShowHint();
std::string GetKeyNameFromScanCode(int scanCode);

#endif  // UTILITY_H
