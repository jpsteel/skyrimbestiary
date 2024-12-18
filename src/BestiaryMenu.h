#ifndef BESTIARY_MENU_H
#define BESTIARY_MENU_H

#include "PCH.h"
#include "RE/Skyrim.h"
#include "RE/G/GFxMovieDef.h"
#include "RE/G/GFxValue.h"
#include "RE/G/GPtr.h"
#include "SKSE/SKSE.h"
#include "Utility.h"

namespace Scaleform {

    class BestiaryMenu : RE::IMenu {
    public:
        static constexpr const char* MENU_PATH = "bestiarymenu";
        static constexpr const char* MENU_NAME = "BestiaryMenu";

        BestiaryMenu();

        static void Register();
        static void Show();
        static void Hide();

        static constexpr std::string_view Name();

        virtual void Accept(RE::FxDelegateHandler::CallbackProcessor* a_cbReg) override;

        static RE::stl::owner<RE::IMenu*> Creator() { return new BestiaryMenu(); }

    private:
        static void PlaySound(const RE::FxDelegateArgs& a_params);
    };

    constexpr std::string_view BestiaryMenu::Name() { return BestiaryMenu::MENU_NAME; }

}

#endif  // BESTIARY_MENU_H
