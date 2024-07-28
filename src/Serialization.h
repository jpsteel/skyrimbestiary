#ifndef SERIALIZATION_H
#define SERIALIZATION_H

#include <SKSE/SKSE.h>

#include "Utility.h"
#include "HUDWidget.h"

constexpr std::uint32_t kHintShown = 'HSHN';

void RevertCallback(SKSE::SerializationInterface*);
void SaveCallback(SKSE::SerializationInterface*);
void LoadCallback(SKSE::SerializationInterface*);
void Serialize(SKSE::SerializationInterface* intfc);
bool Deserialize(SKSE::SerializationInterface* intfc, uint32_t version);

#endif  // SERIALIZATION_H