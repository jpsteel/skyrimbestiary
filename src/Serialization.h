#ifndef SERIALIZATION_H
#define SERIALIZATION_H

#pragma once
#include <SKSE/SKSE.h>

#include "Utility.h"

void RevertCallback(SKSE::SerializationInterface*);
void SaveCallback(SKSE::SerializationInterface*);
void LoadCallback(SKSE::SerializationInterface*);
void Serialize(SKSE::SerializationInterface* intfc);
bool Deserialize(SKSE::SerializationInterface* intfc, uint32_t version);

#endif  // SERIALIZATION_H