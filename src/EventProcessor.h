#ifndef EVENT_PROCESSOR_H
#define EVENT_PROCESSOR_H

#pragma once
#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>

#include "Utility.h"

class EventProcessor : public RE::BSTEventSink<RE::TESActivateEvent>,
                       public RE::BSTEventSink<RE::TESDeathEvent>,
                       public RE::BSTEventSink<RE::TESSpellCastEvent>,
                       public RE::BSTEventSink<RE::TESQuestStageEvent>,
                       public RE::BSTEventSink<RE::MenuOpenCloseEvent> {
public:
    static EventProcessor* GetSingleton() {
        static EventProcessor instance;
        return std::addressof(instance);
    }

    RE::BSEventNotifyControl ProcessEvent(const RE::TESActivateEvent* event,
                                          RE::BSTEventSource<RE::TESActivateEvent>*) override;
    RE::BSEventNotifyControl ProcessEvent(const RE::TESDeathEvent* event,
                                          RE::BSTEventSource<RE::TESDeathEvent>*) override;
    RE::BSEventNotifyControl ProcessEvent(const RE::TESSpellCastEvent* event,
                                          RE::BSTEventSource<RE::TESSpellCastEvent>*) override;
    RE::BSEventNotifyControl ProcessEvent(const RE::TESQuestStageEvent* event,
                                          RE::BSTEventSource<RE::TESQuestStageEvent>*) override;
    RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent* event,
                                          RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override;

private:
    std::vector<std::string> getBestiaryKeywords(RE::TESBoundObject* targetBase);
};

#endif  // EVENT_PROCESSOR_H