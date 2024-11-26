#ifndef PAPYRUSFUNCTIONS_H
#define PAPYRUSFUNCTIONS_H

#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>

bool PapyrusFunctions(RE::BSScript::IVirtualMachine* vm);
bool UnlockEntry(RE::StaticFunctionTag*, RE::BSFixedString creatureKeyword);
void Open(RE::StaticFunctionTag*);
void SetBlocked(RE::StaticFunctionTag*, bool blocked);
bool IsBlocked(RE::StaticFunctionTag*);

#endif  // PAPYRUSFUNCTIONS_H