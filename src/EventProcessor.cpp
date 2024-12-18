#include "EventProcessor.h"
#include "Utility.h"
#include "BestiaryMenu.h"
#include "HUDWidget.h"

RE::BSEventNotifyControl EventProcessor::ProcessEvent(const RE::TESActivateEvent* event,
                                                      RE::BSTEventSource<RE::TESActivateEvent>*) {
    RE::TESBoundObject* actionBase = event->actionRef.get()->GetBaseObject();
    RE::TESBoundObject* targetBase = event->objectActivated.get()->GetBaseObject();
    if (actionBase->IsPlayer()) {
        std::string factionEditorID = "RPGUI_BestiaryAddEntryOnTalkFaction";
        RE::TESForm* recForm = RE::TESForm::LookupByEditorID(factionEditorID);
        RE::TESFaction* recFaction = skyrim_cast<RE::TESFaction*>(recForm);

        if (targetBase->Is(RE::FormType::NPC)) {
            auto actor = event->objectActivated.get()->As<RE::Actor>();
            if (actor->IsInFaction(recFaction)) {
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

    RE::TESBoundObject* victimBase = event->actorDying.get()->GetBaseObject();
    RE::TESBoundObject* killerBase = event->actorKiller.get()->GetBaseObject();

    std::string factionEditorID = "RPGUI_BestiaryAddEntryOnDeathUnique";
    RE::TESForm* recForm = RE::TESForm::LookupByEditorID(factionEditorID);
    RE::TESFaction* recFaction = skyrim_cast<RE::TESFaction*>(recForm);

    auto victimActor = event->actorDying.get()->As<RE::Actor>();
    if (killerBase->IsPlayer() || victimActor->IsInFaction(recFaction)) {
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
        if (spell->GetSpellType() == RE::MagicSystem::SpellType::kSpell) {
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
            auto& data = BestiaryDataMap["EMPEROR CRAB"];
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

    auto ui = RE::UI::GetSingleton();
    if (event->opening) {
        if (event->menuName == Scaleform::BestiaryMenu::MENU_NAME) {
            if (auto menu = ui->GetMenu(Scaleform::BestiaryMenu::MENU_NAME); menu) {
                RE::GFxValue page;
                RE::GFxValue sCurrentCreature;
                ui->GetMenu(Scaleform::BestiaryMenu::MENU_NAME)->uiMovie->GetVariable(&page, "_root.BestiaryMenu_mc");

                std::array<RE::GFxValue, 1> configArgs;
                std::array<RE::GFxValue, 1> lastEntryArgs;
                std::array<RE::GFxValue, 1> listArgs;
                std::array<RE::GFxValue, 1> gamepadArgs;
                std::string creaturesList;
                if (GetGlobalVariableValue("RPGUI_devMode") == 1) {
                    creaturesList = DebugGetAllCreaturesLists();
                } else {
                    creaturesList = GetCreaturesLists();
                }
                int gamepad = RE::BSInputDeviceManager::GetSingleton()->IsGamepadEnabled();
                configArgs[0] = resistanceModConfig;
                lastEntryArgs[0] = lastEntry;
                listArgs[0] = creaturesList;
                gamepadArgs[0] = gamepad;

                page.Invoke("getResistanceMod", nullptr, configArgs.data(), configArgs.size());
                page.Invoke("getGamepadStatus", nullptr, gamepadArgs.data(), gamepadArgs.size());
                page.Invoke("getLastEntry", nullptr, lastEntryArgs.data(), lastEntryArgs.size());
                page.Invoke("getAllCreaturesLists", nullptr, listArgs.data(), listArgs.size());

                return RE::BSEventNotifyControl::kContinue;
            }
        } else if (event->menuName == RE::JournalMenu::MENU_NAME && enableMenuOption == 1) {
            AddMenuOption();
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
    if (!eventPtr || !*eventPtr || !RE::Main::GetSingleton()->gameActive) {
        return RE::BSEventNotifyControl::kContinue;
    }

    auto* event = *eventPtr;
    if (event->eventType == RE::INPUT_EVENT_TYPE::kButton) {
        auto* buttonEvent = event->AsButtonEvent(); 
        auto dxScanCode = buttonEvent->GetIDCode(); 
        auto userEvent = RE::UserEvents::GetSingleton();
        if (buttonEvent->IsDown()) {
            auto ui = RE::UI::GetSingleton();
            if (dxScanCode == menuHotkey && buttonEvent->IsDown()) {
                if (!ui->IsMenuOpen(Scaleform::BestiaryMenu::MENU_NAME) && !ui->GameIsPaused() &&
                    !ui->IsMenuOpen(RE::DialogueMenu::MENU_NAME)) {
                    Scaleform::BestiaryMenu::Show();
                    return RE::BSEventNotifyControl::kContinue;
                } else if (ui->IsMenuOpen(Scaleform::BestiaryMenu::MENU_NAME)) {
                    RE::GFxValue page;
                    ui->GetMenu(Scaleform::BestiaryMenu::MENU_NAME)
                        ->uiMovie->GetVariable(&page, "_root.BestiaryMenu_mc");
                    RE::GFxValue lastEntryGFx;
                    page.Invoke("sendLastEntry", &lastEntryGFx);

                    Scaleform::BestiaryMenu::Hide();
                    if (GetGlobalVariableValue("RPGUI_devMode") == 1) {
                        return RE::BSEventNotifyControl::kContinue;
                    }
                    lastEntry = lastEntryGFx.GetString();
                    logger::trace("Retrieved lastEntry string: {}", lastEntry);
                    return RE::BSEventNotifyControl::kContinue;
                } else {
                    return RE::BSEventNotifyControl::kContinue;
                }
            } else if (buttonEvent->QUserEvent() == userEvent->cancel ||
                       buttonEvent->QUserEvent() == userEvent->tweenMenu) {
                if (ui->IsMenuOpen(Scaleform::BestiaryMenu::MENU_NAME)) {
                    RE::GFxValue page;
                    ui->GetMenu(Scaleform::BestiaryMenu::MENU_NAME)
                        ->uiMovie->GetVariable(&page, "_root.BestiaryMenu_mc");
                    RE::GFxValue lastEntryGFx;
                    page.Invoke("sendLastEntry", &lastEntryGFx);

                    Scaleform::BestiaryMenu::Hide();

                    lastEntry = lastEntryGFx.GetString();
                    logger::trace("Retrieved lastEntry string: {}", lastEntry);

                    return RE::BSEventNotifyControl::kContinue;
                }
            }
        }
    }

    auto ui = RE::UI::GetSingleton();
    if (!ui || !ui->IsMenuOpen(RE::JournalMenu::MENU_NAME) || enableMenuOption != 1) {
        return RE::BSEventNotifyControl::kContinue;
    }

    if (event->eventType == RE::INPUT_EVENT_TYPE::kButton || event->eventType == RE::INPUT_EVENT_TYPE::kThumbstick) {
        const auto menu = ui->GetMenu<RE::JournalMenu>(RE::JournalMenu::MENU_NAME).get();
        const auto view = menu ? menu->GetRuntimeData().systemTab.view : nullptr;
        RE::GFxValue page;
        if (!view || !view->GetVariable(&page, "_root.QuestJournalFader.Menu_mc.SystemFader.Page_mc")) { 
            logger::warn("Couldn't find _root.QuestJournalFader.Menu_mc.SystemFader.Page_mc"); 
            return RE::BSEventNotifyControl::kContinue;
        }

        RE::GFxValue categoryList;
        RE::GFxValue currentState;
        if (page.GetMember("CategoryList", &categoryList) && page.GetMember("iCurrentState", &currentState)) {

            RE::GFxValue selectedIndex; 
            if (categoryList.GetMember("iSelectedIndex", &selectedIndex)) {
                RE::GFxValue entryList;
                if (categoryList.GetMember("entryList", &entryList)) { 
                    RE::GFxValue selectedEntry;
                    entryList.GetElement(selectedIndex.GetNumber(), &selectedEntry);
                    RE::GFxValue entryText;
                    selectedEntry.GetMember("text", &entryText);
                    logger::trace("Entry text: {}", entryText.GetString());
                    logger::trace("Current state: {}", currentState.GetNumber());
                    if (entryText.GetString() == SYSTEMMENU_ALIAS && currentState == 13) {
                        logger::debug("Selected entry is {}", entryText.GetString()); 
                        auto msgQueue = RE::UIMessageQueue::GetSingleton();
                        if (msgQueue) {
                            msgQueue->AddMessage(RE::JournalMenu::MENU_NAME, RE::UI_MESSAGE_TYPE::kHide, nullptr);

                            Scaleform::BestiaryMenu::Show();
                            
                            return RE::BSEventNotifyControl::kContinue;
                        } else {
                            logger::warn("UIMessageQueue singleton not found");
                        }
                        return RE::BSEventNotifyControl::kContinue;
                    }
                }
            } 
        }
        return RE::BSEventNotifyControl::kContinue;
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
                auto [insertIt, inserted] = BestiaryDataMap.emplace(variant, CreatureData{0, 0, 0});
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