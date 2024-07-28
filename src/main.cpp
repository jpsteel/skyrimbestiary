#include <SimpleIni.h>
#include <spdlog/sinks/basic_file_sink.h>

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "EventProcessor.h"
#include "Serialization.h"
#include "Utility.h"
#include "BestiaryMenu.h"
#include "HUDWidget.h"
#include "Sync.h"

void SKSEMessageHandler(SKSE::MessagingInterface::Message* message) {
    auto eventProcessor = EventProcessor::GetSingleton();
    switch (message->type) {

        case (SKSE::MessagingInterface::kDataLoaded):
            RE::ScriptEventSourceHolder::GetSingleton()->AddEventSink<RE::TESActivateEvent>(eventProcessor);
            RE::ScriptEventSourceHolder::GetSingleton()->AddEventSink<RE::TESDeathEvent>(eventProcessor);
            RE::ScriptEventSourceHolder::GetSingleton()->AddEventSink<RE::TESSpellCastEvent>(eventProcessor);
            RE::ScriptEventSourceHolder::GetSingleton()->AddEventSink<RE::TESQuestStageEvent>(eventProcessor);
            RE::UI::GetSingleton()->AddEventSink<RE::MenuOpenCloseEvent>(eventProcessor);
            Scaleform::BestiaryMenu::Register();
            Scaleform::HUDWidget::Register();
            Scaleform::HUDWidget::Show();
            SetResistanceModConfig();
            break;

        case (SKSE::MessagingInterface::kInputLoaded):
            RE::BSInputDeviceManager::GetSingleton()->AddEventSink<RE::InputEvent*>(eventProcessor);
            break;

        case SKSE::MessagingInterface::kPostLoadGame:
        case SKSE::MessagingInterface::kPostLoad:
        case SKSE::MessagingInterface::kNewGame:
        case SKSE::MessagingInterface::kSaveGame:
            Scaleform::HUDWidget::Show();
            break;

        default:
            break;
    }
}



extern "C" [[maybe_unused]] __declspec(dllexport) bool SKSEPlugin_Load(const SKSE::LoadInterface* skse) {
    SKSE::Init(skse);

    SetupLog();
    spdlog::set_level(spdlog::level::debug);

    auto* serialization = SKSE::GetSerializationInterface();
    serialization->SetUniqueID('BSTY');
    serialization->SetSaveCallback(SaveCallback);
    serialization->SetLoadCallback(LoadCallback);
    serialization->SetRevertCallback(RevertCallback);

    SKSE::GetMessagingInterface()->RegisterListener(SKSEMessageHandler);

    LoadDataFromINI();
    PopulateVariantMap();
    std::thread(ProcessQueue).detach();

    logger::info("Bestiary is in Player's pocket!");

    return true;
}