#include "Utility.h"

#include <spdlog/sinks/basic_file_sink.h>

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <nlohmann/json.hpp>
#include "KeyMapping.h"


namespace fs = std::filesystem;
namespace logger = SKSE::log;

int menuHotkey;
int widgetX;
int widgetY;
int enableWidget;
int enableMenuOption;
std::string tutorialMessage;
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
    const char* enable_widget = ini.GetValue("General", "iEnableWidget", "0");
    const char* enable_menu_option = ini.GetValue("General", "iEnableSystemMenuOption", "0");
    const char* tutorial_msg = ini.GetValue("General", "sTutorialMessage", "0");
    menuHotkey = std::stoi(keycodeStr);
    widgetX = std::stoi(widget_x);
    widgetY = std::stoi(widget_y);
    enableWidget = std::stoi(enable_widget);
    enableMenuOption = std::stoi(enable_menu_option);
    tutorialMessage = tutorial_msg;
    logger::debug("Loaded keycode: {}", keycodeStr);
    logger::debug("Loaded widget x offset: {}", widget_x);
    logger::debug("Loaded widget y offset: {}", widget_y);
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
                                    logger::trace("Added variant {} with localized name {} creature {} and category {}",
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

            std::uint32_t index = 0;
            std::string text;
            entryList.VisitMembers([&](const char*, const RE::GFxValue& a_value) {
                RE::GFxValue textVal;
                a_value.GetMember("text", &textVal);
                if (text = textVal.GetString(); text == "$HELP") {
                    bestiaryIndex = index;
                }
                index++;
            });

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