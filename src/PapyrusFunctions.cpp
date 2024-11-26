#include "PapyrusFunctions.h"
#include "Utility.h"
#include "Scaleform.h"
#include "BestiaryMenu.h"
#include <RE/Skyrim.h>
#include <SKSE/Trampoline.h>
#include <SKSE/SKSE.h>
#include <SKSE/API.h>

bool PapyrusFunctions(RE::BSScript::IVirtualMachine* vm) {
    vm->RegisterFunction("UnlockEntry", "Bestiary", UnlockEntry);
    vm->RegisterFunction("Open", "Bestiary", Open);
    vm->RegisterFunction("SetBlocked", "Bestiary", SetBlocked);
    vm->RegisterFunction("IsBlocked", "Bestiary", IsBlocked);
    return true;
}

bool UnlockEntry(RE::StaticFunctionTag*, RE::BSFixedString creatureKeyword) {
    if (creatureKeyword.empty()) {
        logger::warn("[Papyrus] UnlockEntry called with an empty keyword.");
        return false;
    }

    std::string variant = creatureKeyword.c_str();
    std::transform(variant.begin(), variant.end(), variant.begin(), ::toupper);

    auto it = VariantMap.find(variant);
    if (it != VariantMap.end()) {
        auto [creatureName, category, localizedName] = it->second;

        auto [insertIt, inserted] = BestiaryDataMap.emplace(variant, CreatureData{0, 0, 0});
        if (inserted) {
            logger::info("[Papyrus] UnlockEntry: {} added to Bestiary with initial values", variant);

            if (enableWidget == 1) {
                DisplayEntryWithWait(variant);
            }

            CheckAndShowHint();
            return true;
        } else {
            logger::debug("[Papyrus] UnlockEntry: {} is already unlocked in the Bestiary.", variant);
            return true;
        }
    }

    logger::warn("[Papyrus] UnlockEntry called for an unknown variant: {}", variant);
    return false;
}

void Open(RE::StaticFunctionTag*) {
    auto ui = RE::UI::GetSingleton();
    if (!ui->IsMenuOpen(Scaleform::BestiaryMenu::MENU_NAME) && !ui->GameIsPaused() &&
        !ui->IsMenuOpen(RE::DialogueMenu::MENU_NAME)) {
        Scaleform::BestiaryMenu::Show();
        logger::info("[Papyrus] Open: Successfully opened bestiary menu.");
        return;
    } else {
        logger::info("[Papyrus] Open: Couldn't open bestiary menu.");
        return;
    }
}

void SetBlocked(RE::StaticFunctionTag*, bool block) {
    isBestiaryBlocked = block;
    logger::info("[Papyrus] SetBlocked: Bestiary block state set to: {}", block);
}

bool IsBlocked(RE::StaticFunctionTag*) { 
    logger::info("[Papyrus] IsBlocked: {}", isBestiaryBlocked);
    return isBestiaryBlocked; 
}