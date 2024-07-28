#ifndef UTILITY_H
#define UTILITY_H

#include <SimpleIni.h>
#include <spdlog/sinks/basic_file_sink.h>

#include <fstream>
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <condition_variable>
#include "HUDWidget.h"

namespace logger = SKSE::log;

const RE::GFxValue SYSTEMMENU_ALIAS = "$BESTIARY MENU";

extern int menuHotkey;
extern int widgetX;
extern int widgetY;
extern float widgetScale;
extern int enableWidget;
extern int enableMenuOption;
extern std::string tutorialMessage;
extern bool hintShown;
extern std::string resistanceModConfig;

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
extern std::string lastEntry;

void SetupLog();
void LoadDataFromINI();
void PopulateVariantMap();
std::string GetVariantName(const std::string& filePath);
void ShowTutorialHintText();
void HideTutorialHintText();
void CheckAndShowHint();
std::string GetKeyNameFromScanCode(int scanCode);
void AddMenuOption();
void SetResistanceModConfig();
float GetGlobalVariableValue(const std::string& editorID);
void DisplayEntryWithWait(const std::string& variant);
std::string DebugGetAllCreaturesLists();
std::string GetCreaturesLists();
std::string GetTranslatedName(std::string creatureName);
bool CheckIfModIsLoaded(RE::BSFixedString a_modname);

#endif  // UTILITY_H
