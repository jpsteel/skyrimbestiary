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
                      RE::UI_MENU_FLAGS::kAssignCursorToRenderer);
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
        RE::GFxValue widget;
        std::array<RE::GFxValue, 2> posArgs;
        std::array<RE::GFxValue, 1> scaleArgs;
        posArgs[0] = widgetX;
        posArgs[1] = widgetY;
        scaleArgs[0] = widgetScale;
        ui->GetMenu(HUDWidget::MENU_NAME)->uiMovie->GetVariable(&widget, "_root.BestiaryWidget_mc");
        widget.Invoke("setPosition", nullptr, posArgs.data(), posArgs.size());
        widget.Invoke("setScale", nullptr, scaleArgs.data(), scaleArgs.size());

        logger::info("Set widget position to X {} and Y {}, scale to {}", widgetX, widgetY, widgetScale);
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
        auto ui = RE::UI::GetSingleton();
        RE::GFxValue widget;
        if (ui->GetMenu(HUDWidget::MENU_NAME)->uiMovie->GetVariable(&widget, "_root.BestiaryWidget_mc")) {
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
        auto ui = RE::UI::GetSingleton();
        RE::GFxValue widget;  
        if (ui->GetMenu(HUDWidget::MENU_NAME)->uiMovie->GetVariable(&widget, "_root.BestiaryWidget_mc")) { 
            std::array<RE::GFxValue, 1> hideArgs;
            hideArgs[0] = false;  
            widget.Invoke("ShowNotification", nullptr, hideArgs.data(), hideArgs.size());
        }
    }
}
