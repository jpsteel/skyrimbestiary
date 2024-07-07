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
#include "PapyrusFunctions.h"
#include "Utility.h"

void SKSEMessageHandler(SKSE::MessagingInterface::Message* message) {
    if (message->type == SKSE::MessagingInterface::kDataLoaded) {
        auto eventProcessor = EventProcessor::GetSingleton();
        static bool registered = false;
        if (!registered) {
            RE::ScriptEventSourceHolder::GetSingleton()->AddEventSink<RE::TESActivateEvent>(eventProcessor);
            RE::ScriptEventSourceHolder::GetSingleton()->AddEventSink<RE::TESDeathEvent>(eventProcessor);
            RE::ScriptEventSourceHolder::GetSingleton()->AddEventSink<RE::TESSpellCastEvent>(eventProcessor);
            RE::ScriptEventSourceHolder::GetSingleton()->AddEventSink<RE::TESQuestStageEvent>(eventProcessor);
            RE::UI::GetSingleton()->AddEventSink<RE::MenuOpenCloseEvent>(eventProcessor);
            registered = true;
            logger::info("Event processor registered.");
        }
    }
}

extern "C" [[maybe_unused]] __declspec(dllexport) bool SKSEPlugin_Load(const SKSE::LoadInterface* skse) {
    SKSE::Init(skse);

    SetupLog();
    spdlog::set_level(spdlog::level::trace);

    auto* serialization = SKSE::GetSerializationInterface();
    serialization->SetUniqueID('BSTY');
    serialization->SetSaveCallback(SaveCallback);
    serialization->SetLoadCallback(LoadCallback);
    serialization->SetRevertCallback(RevertCallback);

    SKSE::GetPapyrusInterface()->Register(PapyrusFunctions);
    SKSE::GetMessagingInterface()->RegisterListener(SKSEMessageHandler);

    LoadDataFromINI();
    PopulateVariantMap();

    logger::info("Bestiary is in Player's pocket!");

    return true;
}