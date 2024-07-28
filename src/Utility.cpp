#include "Utility.h"

#include <spdlog/sinks/basic_file_sink.h>

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <nlohmann/json.hpp>
#include "KeyMapping.h"
#include "Sync.h"


namespace fs = std::filesystem;
namespace logger = SKSE::log;

std::string lastEntry;
int menuHotkey;
int widgetX;
int widgetY;
float widgetScale;
int enableWidget;
int enableMenuOption;
std::string tutorialMessage;
std::string resistanceModConfig;
bool hintShown = false;
const std::string INI_FILE_PATH = "Data/Dragonborns Bestiary.ini";


std::unordered_map<std::string, VariantInfo> VariantMap;
std::unordered_set<std::string> BestiaryUnlockedCreatures;
std::unordered_map<std::string, CreatureData> BestiaryDataMap;

void SetupLog() {
    auto logsFolder = SKSE::log::log_directory();
    if (!logsFolder) SKSE::stl::report_and_fail("SKSE log_directory not provided, logs disabled.");
    auto pluginName = SKSE::PluginDeclaration::GetSingleton()->GetName();
    auto logFilePath = *logsFolder / std::format("{}.log", pluginName);
    auto fileLoggerPtr = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFilePath.string(), true);
    auto loggerPtr = std::make_shared<spdlog::logger>("log", std::move(fileLoggerPtr));
    spdlog::set_default_logger(std::move(loggerPtr));
    spdlog::set_level(spdlog::level::trace);
    spdlog::flush_on(spdlog::level::trace);
}

void LoadDataFromINI() {
    CSimpleIniA ini;
    ini.SetUnicode();
    SI_Error rc = ini.LoadFile(INI_FILE_PATH.c_str());
    if (rc < 0) {
        logger::error("Failed to load INI file: {}", INI_FILE_PATH);
        return;
    }
 
    const char* keycodeStr = ini.GetValue("General", "iKeycode", "0");
    const char* widget_x = ini.GetValue("General", "iBestiaryWidget_X", "0");
    const char* widget_y = ini.GetValue("General", "iBestiaryWidget_Y", "0");
    const char* widget_scale = ini.GetValue("General", "iBestiaryWidgetScale", "0");
    const char* enable_widget = ini.GetValue("General", "iEnableWidget", "0");
    const char* enable_menu_option = ini.GetValue("General", "iEnableSystemMenuOption", "0");
    const char* tutorial_msg = ini.GetValue("General", "sTutorialMessage", "0");
    menuHotkey = std::stoi(keycodeStr);
    widgetX = std::stoi(widget_x);
    widgetY = std::stoi(widget_y);
    widgetScale = std::stof(widget_scale);
    enableWidget = std::stoi(enable_widget);
    enableMenuOption = std::stoi(enable_menu_option);
    tutorialMessage = tutorial_msg;
    logger::debug("Loaded keycode: {}", keycodeStr);
    logger::debug("Loaded widget x offset: {}", widget_x);
    logger::debug("Loaded widget y offset: {}", widget_y);
    logger::debug("Loaded widget scale: {}", widget_scale);
    logger::debug("Loaded widget enabled: {}", enable_widget);
    logger::debug("Loaded menu option enabled: {}", enable_menu_option);
    logger::debug("Loaded tutorial message localization: {}", tutorial_msg);
}

void PopulateVariantMap() {
    std::string basePath = "DATA/interface/creatures";
    try {
        for (const auto& categoryEntry : fs::directory_iterator(basePath)) {
            if (categoryEntry.is_directory()) {
                std::string category = categoryEntry.path().filename().string();
                for (const auto& creatureEntry : fs::directory_iterator(categoryEntry.path())) {
                    if (creatureEntry.is_directory()) {
                        std::string creatureName = creatureEntry.path().filename().string();
                        for (const auto& variantEntry : fs::directory_iterator(creatureEntry.path())) {
                            if (variantEntry.path().extension() == ".json") {
                                std::string variant = variantEntry.path().stem().string();
                                if (variant.find("_LOOT") == std::string::npos &&
                                    variant.find("_RESIST") == std::string::npos) {
                                    std::transform(variant.begin(), variant.end(), variant.begin(), ::toupper);
                                    std::string localizedName = GetVariantName(variantEntry.path().string());
                                    VariantMap[variant] = {creatureName, category, localizedName};
                                    logger::debug("Added variant {} with localized name {} creature {} and category {}",
                                                  variant, localizedName, creatureName, category);
                                }
                            }
                        }
                    }
                }
            }
        }
    } catch (const fs::filesystem_error& e) {
        logger::error("Filesystem error: {}", e.what());
    } catch (const std::exception& e) {
        logger::error("Error: {}", e.what());
    }
}

std::string GetVariantName(const std::string& filePath) {
    std::ifstream file(filePath);
    nlohmann::json json;
    file >> json;
    return json.value("name", "");
}

void ShowTutorialHintText() {
    auto hud = RE::UI::GetSingleton()->GetMenu(RE::HUDMenu::MENU_NAME);
    if (hud) {
        RE::GFxValue args[2];
        std::string key = GetKeyNameFromScanCode(menuHotkey);

        std::string message = std::vformat(std::string_view{tutorialMessage}, std::make_format_args(key));
        args[0].SetString(message);
        args[1].SetBoolean(true);
        hud->uiMovie->Invoke("_root.HUDMovieBaseInstance.ShowTutorialHintText", nullptr, args, 2);
    }
}

void HideTutorialHintText() {
    std::this_thread::sleep_for(std::chrono::seconds(3));
    auto hud = RE::UI::GetSingleton()->GetMenu(RE::HUDMenu::MENU_NAME);
    if (hud) {
        RE::GFxValue args[2];
        args[0].SetString("");
        args[1].SetBoolean(false);
        hud->uiMovie->Invoke("_root.HUDMovieBaseInstance.ShowTutorialHintText", nullptr, args, 2);
    }
}

void CheckAndShowHint() {
    if (!hintShown) {
        logger::debug("Showing bestiary tutorial");
        ShowTutorialHintText();
        hintShown = true;
        std::jthread t(HideTutorialHintText);
        t.detach();
    }
}

std::string GetKeyNameFromScanCode(int scanCode) {
    auto it = kKeyMap.find(scanCode);
    if (it != kKeyMap.end()) {
        return it->second;
    }
    return "Unknown";
}

void AddMenuOption() {
    const auto menu = RE::UI::GetSingleton()->GetMenu<RE::JournalMenu>(RE::JournalMenu::MENU_NAME).get();
    const auto view = menu ? menu->GetRuntimeData().systemTab.view : nullptr;
    RE::GFxValue page;
    if (!view || !view->GetVariable(&page, "_root.QuestJournalFader.Menu_mc.SystemFader.Page_mc")) {
        logger::warn("Couldn't find _root.QuestJournalFader.Menu_mc.SystemFader.Page_mc");
        return;
    }
    RE::GFxValue showModMenu;
    if (page.GetMember("_showModMenu", &showModMenu) && showModMenu.GetBool() == false) {
        std::array<RE::GFxValue, 1> args;
        args[0] = true;
        if (!page.Invoke("SetShowMod", nullptr, args.data(), args.size())) {
            logger::warn("Couldn't invoke SetShowMod");
            return;
        }
    }
    RE::GFxValue categoryList;
    if (page.GetMember("CategoryList", &categoryList)) {
        RE::GFxValue entryList;
        if (categoryList.GetMember("entryList", &entryList)) {
            std::uint32_t bestiaryIndex;

            std::uint32_t length = entryList.GetArraySize();
            for (std::uint32_t index = 0; index < length; ++index) {
                RE::GFxValue entry;
                if (entryList.GetElement(index, &entry)) {
                    RE::GFxValue textVal;
                    if (entry.GetMember("text", &textVal)) {
                        std::string text = textVal.GetString();
                        if (text == "$HELP") {
                            bestiaryIndex = index;
                            break;
                        }
                    }
                }
            }

            if (bestiaryIndex) {
                RE::GFxValue entryBestiary;
                view->CreateObject(&entryBestiary);
                entryBestiary.SetMember("text", SYSTEMMENU_ALIAS);
                entryList.SetElement(bestiaryIndex, entryBestiary);
                categoryList.Invoke("InvalidateData");

                return;
            }
        }
    }
    return;
}

void SetResistanceModConfig() {
    if (CheckIfModIsLoaded("Resistances and Weaknesses.esp")) { 
        logger::info("Found resistance mod: Resistances & Weaknesses");
        resistanceModConfig = "r&w";
        return;
    } else if (CheckIfModIsLoaded("know_your_enemy2.esp")) {
        logger::info("Found resistance mod: Know Your Enemy 2");
        resistanceModConfig = "kye2";
        return;
    } else if (CheckIfModIsLoaded("Requiem.esp")) {
        logger::info("Found resistance mod: Requiem");
        resistanceModConfig = "req";
        return;
    } else {
        logger::info("No resistance mod found, using Vanilla settings");
        resistanceModConfig = "vanilla";
        return;
    }
}

bool CheckIfModIsLoaded(RE::BSFixedString a_modname) {
    auto* dataHandler = RE::TESDataHandler::GetSingleton();
    const RE::TESFile* modInfo = dataHandler->LookupModByName(a_modname.data());
    if (!modInfo || modInfo->compileIndex == 255) {  
        return false;
    }
    return true;
}

float GetGlobalVariableValue(const std::string& editorID) {
    auto dataHandler = RE::TESDataHandler::GetSingleton();
    if (!dataHandler) {
        logger::warn("TESDataHandler not found.");
        return 0.0f;
    }

    for (auto& global : dataHandler->GetFormArray<RE::TESGlobal>()) {
        if (global->GetFormEditorID() == editorID) {
            return global->value;
        }
    }

    logger::warn("Global variable with EditorID %s not found.", editorID.c_str());
    return 0.0f;
}

void DisplayEntryWithWait(const std::string& variant) {
    std::lock_guard<std::mutex> lock(entryMutex);
    entryQueue.push(variant);
}

std::string DebugGetAllCreaturesLists() {
    logger::info("Writing full creatures list (dev mode)");
    std::unordered_map<std::string, std::unordered_map<std::string, std::vector<std::string>>> categoryCreatureMap;

    for (const auto& [variant, info] : VariantMap) {
        categoryCreatureMap[info.category][info.creature].push_back(variant + "#0#0#0");
    }

    std::ostringstream creaturesListFull;

    for (const auto& [category, creatures] : categoryCreatureMap) {
        std::ostringstream creaturesListJoin;
        for (const auto& [creatureName, variants] : creatures) {
            std::ostringstream variantsListJoin;
            for (const auto& variant : variants) {
                variantsListJoin << variant << "@";
            }
            std::string variantsStr = variantsListJoin.str();
            if (!variantsStr.empty()) {
                variantsStr.pop_back();  // Remove the last '@'
                creaturesListJoin << creatureName << "_" << variantsStr << ",";
            }
        }
        std::string creaturesStr = creaturesListJoin.str();
        if (!creaturesStr.empty()) {
            creaturesStr.pop_back();  // Remove the last ','
            creaturesListFull << category << ":" << creaturesStr << "&";
        }
    }

    std::string result = creaturesListFull.str();
    if (!result.empty()) {
        result.pop_back();  // Remove the last '&'
    }

    return result;
}

std::string GetCreaturesLists() {
    logger::info("Writing unlocked creatures list");
    std::unordered_map<std::string, std::unordered_map<std::string, std::vector<std::string>>> categoryCreatureMap;

    for (const auto& [variant, info] : VariantMap) {
        if (BestiaryDataMap.find(variant) != BestiaryDataMap.end()) {
            const CreatureData& data = BestiaryDataMap[variant];
            categoryCreatureMap[info.category][info.creature].push_back(variant + "#" + std::to_string(data.kills) +
                                                                        "#" + std::to_string(data.summons) + "#" +
                                                                        std::to_string(data.transformations));
        }
    }

    std::ostringstream creaturesListFull;

    for (const auto& [category, creatures] : categoryCreatureMap) {
        std::ostringstream creaturesListJoin;
        for (const auto& [creatureName, variants] : creatures) {
            std::ostringstream variantsListJoin;
            for (const auto& variant : variants) {
                variantsListJoin << variant << "@";
            }
            std::string variantsStr = variantsListJoin.str();
            if (!variantsStr.empty()) {
                variantsStr.pop_back();  // Remove the last '@'
                creaturesListJoin << creatureName << "_" << variantsStr << ",";
            }
        }
        std::string creaturesStr = creaturesListJoin.str();
        if (!creaturesStr.empty()) {
            creaturesStr.pop_back();  // Remove the last ','
            creaturesListFull << category << ":" << creaturesStr << "&";
        }
    }

    std::string result = creaturesListFull.str();
    if (!result.empty()) {
        result.pop_back();  // Remove the last '&'
    }

    return result;
}

std::string GetTranslatedName(std::string creatureName) {
    std::transform(creatureName.begin(), creatureName.end(), creatureName.begin(), ::toupper);
    auto it = VariantMap.find(creatureName);
    if (it != VariantMap.end()) {
        std::string locName = it->second.localizedName;
        return locName;
    } else {
        logger::warn("Creature name {} not found in VariantMap", creatureName);
        return creatureName;
    }
}