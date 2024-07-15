#pragma once
#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>

bool PapyrusFunctions(RE::BSScript::IVirtualMachine* vm);

std::string debugGetAllCreaturesLists(RE::StaticFunctionTag*);
std::string getCreaturesLists(RE::StaticFunctionTag*);
std::string getTranslatedName(RE::StaticFunctionTag*, RE::BSFixedString creatureName);
int getMenuHotkey(RE::StaticFunctionTag*);
int getWidgetX(RE::StaticFunctionTag*);
int getWidgetY(RE::StaticFunctionTag*);
int getEnableWidget(RE::StaticFunctionTag*);