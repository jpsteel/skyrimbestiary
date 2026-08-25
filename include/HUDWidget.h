#ifndef HUD_WIDGET_H
#define HUD_WIDGET_H

#include "PCH.h"
#include "RE/Skyrim.h"
#include "RE/G/GFxMovieDef.h"
#include "RE/G/GFxValue.h"
#include "RE/G/GPtr.h"
#include "SKSE/SKSE.h"
#include "Utility.h"


namespace Scaleform {

    class HUDWidget : RE::IMenu {
    public:
        static constexpr const char* MENU_PATH = "exported/widgets/bestiary/bestiarywidget";
        static constexpr const char* MENU_NAME = "BestiaryWidget";

        HUDWidget();

        static void Register();
        static void Show();
        static void Hide();
        static void DisplayEntry(std::string variant);
        static void HideEntry();
        static void SetPosition();

        static constexpr std::string_view Name();

        virtual void Accept(RE::FxDelegateHandler::CallbackProcessor* a_cbReg) override;

        static RE::stl::owner<RE::IMenu*> Creator() { return new HUDWidget(); }

    private:
        static void PlaySound(const RE::FxDelegateArgs& a_params);
    };

    constexpr std::string_view HUDWidget::Name() { return HUDWidget::MENU_NAME; }

}

#endif  // HUD_WIDGET_H