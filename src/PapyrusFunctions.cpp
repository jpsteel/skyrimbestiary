#include "PapyrusFunctions.h"
#include "Utility.h"

bool PapyrusFunctions(RE::BSScript::IVirtualMachine* vm) {
    vm->RegisterFunction("debugGetAllCreaturesLists", "RPGUI_BestiaryQuestScript", debugGetAllCreaturesLists);
    vm->RegisterFunction("getCreaturesLists", "RPGUI_BestiaryQuestScript", getCreaturesLists);
    vm->RegisterFunction("getMenuHotkey", "RPGUI_BestiaryQuestScript", getMenuHotkey);
    vm->RegisterFunction("getWidgetX", "RPGUI_BestiaryQuestScript", getWidgetX);
    vm->RegisterFunction("getWidgetY", "RPGUI_BestiaryQuestScript", getWidgetY);
    return true;
}

std::string debugGetAllCreaturesLists(RE::StaticFunctionTag*) {
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

std::string getCreaturesLists(RE::StaticFunctionTag*) {
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

int getMenuHotkey(RE::StaticFunctionTag*) { return menuHotkey; }

int getWidgetX(RE::StaticFunctionTag*) { return widgetX; }

int getWidgetY(RE::StaticFunctionTag*) { return widgetY; }