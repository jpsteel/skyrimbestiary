#include "BestiaryMenu.h"
#include "Scaleform.h"
#include "Utility.h"

static bool IsHelgenIntroLocked() {
    auto* quest = RE::TESForm::LookupByEditorID<RE::TESQuest>("MQ101");
    if (!quest) {
        return false;
    }

    return quest->IsRunning() && quest->GetCurrentStageID() > 0 &&
        quest->GetCurrentStageID() < 250;
}

namespace Scaleform {
    BestiaryMenu::BestiaryMenu() {
        auto scaleformManager = RE::BSScaleformManager::GetSingleton();
        scaleformManager->LoadMovieEx(this, MENU_PATH, [this](RE::GFxMovieDef* a_def) {
            using StateType = RE::GFxState::StateType;

            fxDelegate.reset(new RE::FxDelegate());
            fxDelegate->RegisterHandler(this);
            a_def->SetState(StateType::kExternalInterface, fxDelegate.get());
            fxDelegate->Release();

            auto logger = new Logger<BestiaryMenu>();
            a_def->SetState(StateType::kLog, logger);
            logger->Release();
        });

        inputContext = Context::kMenuMode;
        depthPriority = 3;
        menuFlags.set(RE::UI_MENU_FLAGS::kPausesGame, RE::UI_MENU_FLAGS::kDisablePauseMenu,
                      RE::UI_MENU_FLAGS::kUsesBlurredBackground, RE::UI_MENU_FLAGS::kModal,
                      RE::UI_MENU_FLAGS::kUsesMenuContext, RE::UI_MENU_FLAGS::kTopmostRenderedMenu,
                      RE::UI_MENU_FLAGS::kUsesMovementToDirection);

        if (!RE::BSInputDeviceManager::GetSingleton()->IsGamepadEnabled()) {
            menuFlags |= RE::UI_MENU_FLAGS::kUsesCursor;
        }
    }

    void BestiaryMenu::Register() {
        auto ui = RE::UI::GetSingleton();
        if (ui) {
            ui->Register(BestiaryMenu::MENU_NAME, Creator);
            logger::debug("Registered {}", BestiaryMenu::MENU_NAME);
        }
    }

    void BestiaryMenu::Show() {
        auto uiMessageQueue = RE::UIMessageQueue::GetSingleton();
        if (isBestiaryBlocked) {
            logger::warn("Blocked attempt to open Bestiary, isBestiaryBlocked set to True.");
            return;
        }

        if (IsHelgenIntroLocked()) {
            logger::warn("Blocked attempt to open Bestiary during intro quest.");
            return;
        }

        if (uiMessageQueue) {
            uiMessageQueue->AddMessage(BestiaryMenu::MENU_NAME, RE::UI_MESSAGE_TYPE::kShow, nullptr);
            RE::UIBlurManager::GetSingleton()->IncrementBlurCount();
        }
    }

    void BestiaryMenu::Hide() {
        auto uiMessageQueue = RE::UIMessageQueue::GetSingleton();
        if (uiMessageQueue) {
            uiMessageQueue->AddMessage(BestiaryMenu::MENU_NAME, RE::UI_MESSAGE_TYPE::kHide, nullptr);
            RE::PlaySound("UIJournalClose");
            RE::UIBlurManager::GetSingleton()->DecrementBlurCount();
        }
    }

    void BestiaryMenu::Accept(RE::FxDelegateHandler::CallbackProcessor* a_cbReg) {
        a_cbReg->Process("PlaySound", PlaySound);
        a_cbReg->Process("CloseMenu", CloseMenu);
        a_cbReg->Process("MarkEntryRead", MarkEntryRead);
    }

    void BestiaryMenu::PlaySound(const RE::FxDelegateArgs& a_params) {
        assert(a_params.GetArgCount() == 1);
        assert(a_params[0].IsString());

        RE::PlaySound(a_params[0].GetString());
    }

    void BestiaryMenu::CloseMenu(const RE::FxDelegateArgs& args) {
        auto* ui = RE::UI::GetSingleton();
        if (!ui || !ui->IsMenuOpen(MENU_NAME) ||
            ui->IsMenuOpen(RE::TutorialMenu::MENU_NAME)) {
            return;
        }

        if (args.GetArgCount() == 1 && args[0].IsString()) {
            lastEntry = args[0].GetString();
        }

        Hide();
    }

    void BestiaryMenu::MarkEntryRead(const RE::FxDelegateArgs& args) {
        if (args.GetArgCount() != 1 || !args[0].IsString()) {
            return;
        }

        const auto entry = BestiaryDataMap.find(args[0].GetString());
        if (entry != BestiaryDataMap.end()) {
            entry->second.unread = false;
        }
    }
}
