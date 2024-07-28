#include "BestiaryMenu.h"
#include "Scaleform.h"
#include "Utility.h"

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
    }

    void BestiaryMenu::PlaySound(const RE::FxDelegateArgs& a_params) {
        assert(a_params.GetArgCount() == 1);
        assert(a_params[0].IsString());

        RE::PlaySound(a_params[0].GetString());
    }
}
