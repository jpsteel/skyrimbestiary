#pragma once
#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>

bool PapyrusFunctions(RE::BSScript::IVirtualMachine* vm);

std::string debugGetAllCreaturesLists(RE::StaticFunctionTag*);
std::string getCreaturesLists(RE::StaticFunctionTag*);
int getMenuHotkey(RE::StaticFunctionTag*);
int getWidgetX(RE::StaticFunctionTag*);
int getWidgetY(RE::StaticFunctionTag*);