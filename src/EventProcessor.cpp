#include "EventProcessor.h"
#include "Utility.h"
#include "BestiaryMenu.h"
#include "HUDWidget.h"

RE::BSEventNotifyControl EventProcessor::ProcessEvent(const RE::TESActivateEvent* event,
                                                      RE::BSTEventSource<RE::TESActivateEvent>*) {
    auto actionRef = event->actionRef.get();
    auto objectActivated = event->objectActivated.get();
    if (!actionRef || !objectActivated) {
        return RE::BSEventNotifyControl::kContinue;
    }

    auto* actionBase = actionRef->GetBaseObject();
    auto* targetBase = objectActivated->GetBaseObject();
    if (!actionBase || !targetBase) {
        return RE::BSEventNotifyControl::kContinue;
    }
    if (actionBase->IsPlayer()) {
        std::string factionEditorID = "RPGUI_BestiaryAddEntryOnTalkFaction";
        RE::TESForm* recForm = RE::TESForm::LookupByEditorID(factionEditorID);
        RE::TESFaction* recFaction = skyrim_cast<RE::TESFaction*>(recForm);

        if (targetBase->Is(RE::FormType::NPC)) {
            auto actor = objectActivated->As<RE::Actor>();
            if (actor && recFaction && actor->IsInFaction(recFaction)) {
                logger::debug("Player activated {}, who is a member of RPGUI_BestiaryAddEntryOnTalkFaction faction",
                              targetBase->GetName());
                getBestiaryKeywords(targetBase);
            }
        }
    }
    return RE::BSEventNotifyControl::kContinue;
}

RE::BSEventNotifyControl EventProcessor::ProcessEvent(const RE::TESDeathEvent* event,
                                                      RE::BSTEventSource<RE::TESDeathEvent>*) {
    auto victim = event->actorDying.get();
    auto killer = event->actorKiller.get();
    uint8_t state = event->dead;

    if (!victim || !killer || state == 1) {
        return RE::BSEventNotifyControl::kContinue;
    }

    auto* victimBase = victim->GetBaseObject();
    auto* killerBase = killer->GetBaseObject();
    if (!victimBase || !killerBase) {
        return RE::BSEventNotifyControl::kContinue;
    }

    std::string factionEditorID = "RPGUI_BestiaryAddEntryOnDeathUnique";
    RE::TESForm* recForm = RE::TESForm::LookupByEditorID(factionEditorID);
    RE::TESFaction* recFaction = skyrim_cast<RE::TESFaction*>(recForm);

    auto victimActor = victim->As<RE::Actor>();
    if (killerBase->IsPlayer() || (recFaction && victimActor && victimActor->IsInFaction(recFaction))) {
        logger::debug("Player killed {}", victimBase->GetName());
        auto addedVariants = getBestiaryKeywords(victimBase);
        for (const auto& variant : addedVariants) {
            auto& data = BestiaryDataMap[variant];
            data.kills++;
            logger::info("Incremented kill count for {}: {}", variant, data.kills);

            SKSE::ModCallbackEvent modEvent("IncrementCounter");
            SKSE::GetModCallbackEventSource()->SendEvent(&modEvent);
        }
    }

    return RE::BSEventNotifyControl::kContinue;
}

RE::BSEventNotifyControl EventProcessor::ProcessEvent(const RE::TESSpellCastEvent* event,
                                                      RE::BSTEventSource<RE::TESSpellCastEvent>*) {
    if (event) {
        logger::trace("TESSpellCastEvent received");
    } else {
        logger::trace("TESSpellCastEvent is null");
        return RE::BSEventNotifyControl::kContinue;
    }

    auto caster = event->object;
    if (!caster || !caster->IsPlayerRef()) {
        logger::trace("Caster is not the player");
        return RE::BSEventNotifyControl::kContinue;
    }

    auto spell = RE::TESForm::LookupByID<RE::SpellItem>(event->spell);
    if (!spell) {
        logger::trace("Spell not found");
        return RE::BSEventNotifyControl::kContinue;
    }

    logger::debug("Player cast {}", spell->GetName());
    auto addedVariants = getBestiaryKeywords(spell);
    for (const auto& variant : addedVariants) {
        auto& data = BestiaryDataMap[variant];
        if (spell->GetSpellType() == RE::MagicSystem::SpellType::kSpell || spell->GetSpellType() == RE::MagicSystem::SpellType::kScroll) {
            data.summons++;
            logger::info("Incremented summon count for {}: {}", variant, data.summons);

            SKSE::ModCallbackEvent modEvent("IncrementCounter", variant);
            SKSE::GetModCallbackEventSource()->SendEvent(&modEvent);
        } else if (spell->GetSpellType() == RE::MagicSystem::SpellType::kPower ||
                   spell->GetSpellType() == RE::MagicSystem::SpellType::kLesserPower) {
            data.transformations++;
            logger::info("Incremented transformation count for {}: {}", variant, data.transformations);

            SKSE::ModCallbackEvent modEvent("IncrementCounter", variant);
            SKSE::GetModCallbackEventSource()->SendEvent(&modEvent);
        }
    }

    return RE::BSEventNotifyControl::kContinue;
}

RE::BSEventNotifyControl EventProcessor::ProcessEvent(const RE::TESQuestStageEvent* event,
                                                      RE::BSTEventSource<RE::TESQuestStageEvent>*) {
    static RE::FormID WaveBreakerQuestID = 0;
    static bool initialized = false;

    if (!initialized) {
        static const std::string WaveBreakerQuestEditorID = "ccBGSSSE001_Crab_MQ4";
        auto form = RE::TESForm::LookupByEditorID(WaveBreakerQuestEditorID);
        if (form) {
            WaveBreakerQuestID = form->GetFormID();
            logger::info("Initialized WaveBreakerQuestID: {:X}", WaveBreakerQuestID);
        } else {
            logger::error("Failed to find form with EditorID: {}", WaveBreakerQuestEditorID);
        }
        initialized = true;
    }

    if (event->formID == WaveBreakerQuestID) {
        if (event->stage == 500) {
            logger::info("Player killed Emperor Crab");
            auto& data = BestiaryDataMap.try_emplace("EMPEROR CRAB", CreatureData{ 0, 0, 0, true }).first->second;
            data.kills++;
            logger::info("Incremented kill count for Emperor Crab: {}", data.kills);

            SKSE::ModCallbackEvent modEvent("IncrementCounter");
            SKSE::GetModCallbackEventSource()->SendEvent(&modEvent);
        }
    }

    return RE::BSEventNotifyControl::kContinue;
}

RE::BSEventNotifyControl EventProcessor::ProcessEvent(const RE::MenuOpenCloseEvent* event,
                                                      RE::BSTEventSource<RE::MenuOpenCloseEvent>*) {
    if (!event) {
        return RE::BSEventNotifyControl::kContinue;
    }

    if (event->opening) {
        if (event->menuName == Scaleform::BestiaryMenu::MENU_NAME) {
            auto* tasks = SKSE::GetTaskInterface();
            if (!tasks) {
                return RE::BSEventNotifyControl::kContinue;
            }

            tasks->AddUITask([] {
                auto* ui = RE::UI::GetSingleton();
                if (!ui) {
                    return;
                }

                auto menu = ui->GetMenu(Scaleform::BestiaryMenu::MENU_NAME);
                if (!menu || !menu->uiMovie) {
                    return;
                }

                RE::GFxValue page;
                if (!menu->uiMovie->GetVariable(&page, "_root.BestiaryMenu_mc") || !page.IsObject()) {
                    return;
                }

                //Screen size adaptation///////
                UpdateUltraWide(*menu->uiMovie.get());

                RE::GFxValue arg;
                arg.SetBoolean(g_isUltraWide);

                menu->uiMovie->Invoke("_root.BestiaryMenu_mc.SetWidescreen", nullptr, &arg, 1);
                ////////////////////////////////

                std::array<RE::GFxValue, 1> configArgs;
                std::array<RE::GFxValue, 1> lastEntryArgs;
                std::array<RE::GFxValue, 1> listArgs;
                std::array<RE::GFxValue, 1> gamepadArgs;
                const std::string creaturesList = GetGlobalVariableValue("RPGUI_devMode") == 1
                    ? DebugGetAllCreaturesLists()
                    : GetCreaturesLists();
                auto* input = RE::BSInputDeviceManager::GetSingleton();
                const int gamepad = input && input->IsGamepadEnabled();

                configArgs[0] = resistanceModConfig;
                lastEntryArgs[0] = lastEntry;
                listArgs[0] = creaturesList;
                gamepadArgs[0] = gamepad;

                page.Invoke("getResistanceMod", nullptr, configArgs.data(), configArgs.size());
                page.Invoke("getGamepadStatus", nullptr, gamepadArgs.data(), gamepadArgs.size());
                page.Invoke("getLastEntry", nullptr, lastEntryArgs.data(), lastEntryArgs.size());
                page.Invoke("getAllCreaturesLists", nullptr, listArgs.data(), listArgs.size());

                if (auto* message = RE::TESForm::LookupByEditorID<RE::BGSMessage>("HelpBestiaryShort")) {
                    RE::TutorialMenu::OpenMenu(message);
                }
                });
            return RE::BSEventNotifyControl::kContinue;
        } else if (event->menuName == RE::BookMenu::MENU_NAME) {
            auto book = RE::BookMenu::GetTargetForm();
            if (book) {
                auto addedVariants = getBestiaryKeywords(book);
                for (const auto& variant : addedVariants) {
                    logger::info("Player read book and added {}", variant);
                }
            }
            return RE::BSEventNotifyControl::kContinue;
        } else if (event->menuName == Scaleform::HUDWidget::MENU_NAME) {
            Scaleform::HUDWidget::SetPosition();
        } else {
            return RE::BSEventNotifyControl::kContinue;
        }
    } else if (!event->opening && event->menuName == Scaleform::HUDWidget::MENU_NAME) {
        Scaleform::HUDWidget::Show();
    }
    return RE::BSEventNotifyControl::kContinue;
}

RE::BSEventNotifyControl EventProcessor::ProcessEvent(RE::InputEvent* const* eventPtr,
                                                      RE::BSTEventSource<RE::InputEvent*>*) {
    auto* main = RE::Main::GetSingleton();
    if (!eventPtr || !*eventPtr || !main || !main->GetRuntimeData().gameActive) {
        return RE::BSEventNotifyControl::kContinue;
    }

    if (auto* ui = RE::UI::GetSingleton();
        ui && ui->IsMenuOpen(Scaleform::BestiaryMenu::MENU_NAME) &&
        !ui->IsMenuOpen(RE::TutorialMenu::MENU_NAME)) {
        for (auto* input = *eventPtr; input; input = input->next) {
            const auto* stick = input->AsThumbstickEvent();
            if (!stick || stick->GetDevice() != RE::INPUT_DEVICE::kGamepad ||
                !stick->IsRight()) {
                continue;
            }

            if (auto menu = ui->GetMenu(Scaleform::BestiaryMenu::MENU_NAME);
                menu && menu->uiMovie) {
                RE::GFxValue args[] = {
                    RE::GFxValue(static_cast<double>(stick->xValue)),
                    RE::GFxValue(static_cast<double>(stick->yValue))
                };
                menu->uiMovie->Invoke("_root.BestiaryMenu_mc.onRightStickInput", nullptr, args, 2);
            }
        }
    }

    auto* event = *eventPtr;
    if (event->eventType == RE::INPUT_EVENT_TYPE::kButton) {
        auto* buttonEvent = event->AsButtonEvent(); 
        auto keyCode = buttonEvent->GetIDCode();
        if (buttonEvent->GetDevice() == RE::INPUT_DEVICE::kGamepad) {
            keyCode = SKSE::InputMap::GamepadMaskToKeycode(keyCode);
        }
        auto userEvent = RE::UserEvents::GetSingleton();
        if (buttonEvent->IsDown()) {
            auto* ui = RE::UI::GetSingleton();
            if (!ui || ui->IsMenuOpen(RE::TutorialMenu::MENU_NAME)) {
                return RE::BSEventNotifyControl::kContinue;
            }
            if (keyCode == menuHotkey) {
                if (!ui->IsMenuOpen(Scaleform::BestiaryMenu::MENU_NAME) && !ui->GameIsPaused() &&
                    !ui->IsMenuOpen(RE::DialogueMenu::MENU_NAME) && !ui->IsMenuOpen(RE::CraftingMenu::MENU_NAME)) {
                    Scaleform::BestiaryMenu::Show();
                    return RE::BSEventNotifyControl::kContinue;
                } else if (ui->IsMenuOpen(Scaleform::BestiaryMenu::MENU_NAME)) {
                    if (auto menu = ui->GetMenu(Scaleform::BestiaryMenu::MENU_NAME);
                        menu && menu->uiMovie) {
                        menu->uiMovie->Invoke("_root.BestiaryMenu_mc.CloseMenu", nullptr, nullptr, 0);
                    }
                    return RE::BSEventNotifyControl::kContinue;
                } else {
                    return RE::BSEventNotifyControl::kContinue;
                }
            }
        }
    }
    return RE::BSEventNotifyControl::kContinue;
}

std::vector<std::string> EventProcessor::getBestiaryKeywords(RE::TESBoundObject* creature) {
    std::vector<std::string> addedVariants;
    auto keywordForm = creature->As<RE::BGSKeywordForm>();
    if (keywordForm) {
        std::unordered_set<std::string> variantsToAdd;

        for (uint32_t i = 0; i < keywordForm->numKeywords; ++i) {
            RE::BGSKeyword* keyword = keywordForm->keywords[i];
            std::string kwrd = keyword->GetFormEditorID();
            if (kwrd.find("Bestiary") != std::string::npos) {
                std::string variant = kwrd.substr(8);
                std::transform(variant.begin(), variant.end(), variant.begin(), ::toupper);
                variantsToAdd.insert(variant);
            }
        }

        for (const auto& variant : variantsToAdd) {
            auto it = VariantMap.find(variant);
            if (it != VariantMap.end()) {
                logger::debug("Found bestiary keyword: {}", variant);
                auto [creatureName, category, localizedName] = it->second;
                auto [insertIt, inserted] = BestiaryDataMap.emplace(variant, CreatureData{0, 0, 0, true});
                if (inserted) {
                    logger::info("{} added to Bestiary with initial values", variant);
                    if (enableWidget == 1) {
                        DisplayEntryWithWait(variant);
                    }
                    CheckAndShowHint();
                } else {
                    logger::debug("{} already exists in Bestiary", variant);
                }
                addedVariants.push_back(variant);
            }
        }
    }
    return addedVariants;
}