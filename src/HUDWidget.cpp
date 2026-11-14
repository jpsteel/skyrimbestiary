#include "Scaleform.h"
#include "HUDWidget.h"
#include "Utility.h"
#include "Sync.h"

namespace Scaleform {
    HUDWidget::HUDWidget() {
        auto scaleformManager = RE::BSScaleformManager::GetSingleton();
        scaleformManager->LoadMovieEx(this, MENU_PATH, [this](RE::GFxMovieDef* a_def) {
            using StateType = RE::GFxState::StateType;

            fxDelegate.reset(new RE::FxDelegate());
            fxDelegate->RegisterHandler(this);
            a_def->SetState(StateType::kExternalInterface, fxDelegate.get());
            fxDelegate->Release();

            auto logger = new Logger<HUDWidget>();
            a_def->SetState(StateType::kLog, logger);
            logger->Release();
        });

        inputContext = Context::kNone;
        depthPriority = 0;
        menuFlags.set(RE::UI_MENU_FLAGS::kAllowSaving, RE::UI_MENU_FLAGS::kCustomRendering,
                      RE::UI_MENU_FLAGS::kAssignCursorToRenderer, RE::UI_MENU_FLAGS::kAlwaysOpen);
    }

    void HUDWidget::Register() {
        auto ui = RE::UI::GetSingleton();
        if (ui) {
            ui->Register(HUDWidget::MENU_NAME, Creator);
            logger::debug("Registered {}", HUDWidget::MENU_NAME);
        }
    }

    void HUDWidget::Show() {
        auto ui = RE::UI::GetSingleton();
        if (!ui || ui->IsMenuOpen(HUDWidget::MENU_NAME)) {
            return;
        }
        auto uiMessageQueue = RE::UIMessageQueue::GetSingleton();
        if (uiMessageQueue) {
            uiMessageQueue->AddMessage(HUDWidget::MENU_NAME, RE::UI_MESSAGE_TYPE::kShow, nullptr);
        }
    }

    void HUDWidget::Hide() {
        auto uiMessageQueue = RE::UIMessageQueue::GetSingleton();
        if (uiMessageQueue) {
            uiMessageQueue->AddMessage(HUDWidget::MENU_NAME, RE::UI_MESSAGE_TYPE::kHide, nullptr);
        }
    }

    void HUDWidget::SetPosition() {
        auto ui = RE::UI::GetSingleton();
        if (!ui || !ui->IsMenuOpen(HUDWidget::MENU_NAME)) {
            logger::debug("Bestiary Widget not open, couldn't set position");
            return;
        }
        auto menu = ui->GetMenu(HUDWidget::MENU_NAME);
        if (!menu || !menu->uiMovie) {
            return;
        }

        RE::GFxValue widget;
        if (!menu->uiMovie->GetVariable(&widget, "_root.BestiaryWidget_mc") ||
            !widget.IsObject()) {
            return;
        }

        UpdateUltraWide(*menu->uiMovie.get());

        std::array<RE::GFxValue, 2> position;
        position[0] = g_isUltraWide ? 1570 : 1360;
        position[1] = 250;
        widget.Invoke("setPosition", nullptr, position.data(), position.size());
    }

    void HUDWidget::Accept(RE::FxDelegateHandler::CallbackProcessor* a_cbReg) {
        a_cbReg->Process("PlaySound", PlaySound);
    }

    void HUDWidget::PlaySound(const RE::FxDelegateArgs& a_params) {
        assert(a_params.GetArgCount() == 1);
        assert(a_params[0].IsString());

        RE::PlaySound(a_params[0].GetString());
    }

    void HUDWidget::DisplayEntry(std::string variant) {
        auto* ui = RE::UI::GetSingleton();
        if (!ui) {
            return;
        }

        auto menu = ui->GetMenu(HUDWidget::MENU_NAME);
        RE::GFxValue widget;
        if (menu && menu->uiMovie && menu->uiMovie->GetVariable(&widget, "_root.BestiaryWidget_mc") &&
            widget.IsObject()) {
            std::array<RE::GFxValue, 1> labelArgs;
            std::array<RE::GFxValue, 1> showArgs;
            std::string label = GetTranslatedName(variant); 
            labelArgs[0] = label;
            showArgs[0] = true;
            widget.Invoke("getLabelText", nullptr, labelArgs.data(), labelArgs.size());
            widget.Invoke("ShowNotification", nullptr, showArgs.data(), showArgs.size());
            std::jthread t(HideEntry); 
            t.detach(); 
        }
    }

    void HUDWidget::HideEntry() { 
        std::this_thread::sleep_for(std::chrono::seconds(3));
        auto* tasks = SKSE::GetTaskInterface();
        if (!tasks) {
            return;
        }

        tasks->AddUITask([] {
            auto* ui = RE::UI::GetSingleton();
            if (!ui) {
                return;
            }

            auto menu = ui->GetMenu(HUDWidget::MENU_NAME);
            RE::GFxValue widget;
            if (menu && menu->uiMovie &&
                menu->uiMovie->GetVariable(&widget, "_root.BestiaryWidget_mc") &&
                widget.IsObject()) {
                std::array<RE::GFxValue, 1> hideArgs;
                hideArgs[0] = false;
                widget.Invoke("ShowNotification", nullptr, hideArgs.data(), hideArgs.size());
            }
        });
    }
}
