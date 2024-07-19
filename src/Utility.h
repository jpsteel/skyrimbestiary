#ifndef UTILITY_H
#define UTILITY_H

#include <SimpleIni.h>
#include <spdlog/sinks/basic_file_sink.h>

#include <fstream>
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace logger = SKSE::log;

const RE::GFxValue SYSTEMMENU_ALIAS = "$BESTIARY MENU";

extern int menuHotkey;
extern int widgetX;
extern int widgetY;
extern int enableWidget;
extern int enableMenuOption;
extern std::string tutorialMessage;
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

void SetupLog();
void LoadDataFromINI();
void PopulateVariantMap();
std::string GetVariantName(const std::string& filePath);
void ShowTutorialHintText();
void HideTutorialHintText();
void CheckAndShowHint();
std::string GetKeyNameFromScanCode(int scanCode);
void AddMenuOption();

#endif  // UTILITY_H
