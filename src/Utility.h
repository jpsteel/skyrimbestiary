#ifndef UTILITY_H
#define UTILITY_H

#pragma once

#include <SimpleIni.h>
#include <spdlog/sinks/basic_file_sink.h>

#include <string>
#include <unordered_map>
#include <unordered_set>

namespace logger = SKSE::log;

extern int menuHotkey;
extern int widgetX;
extern int widgetY;

struct VariantInfo {
    std::string creature;
    std::string category;
};

struct CreatureData {
    int kills = 0;
    int summons = 0;
    int transformations = 0;

    void Serialize(SKSE::SerializationInterface* intfc) const;
    bool Deserialize(SKSE::SerializationInterface* intfc);
};

extern std::unordered_map<std::string, VariantInfo> VariantMap;
extern std::unordered_set<std::string> BestiaryUnlockedCreatures;
extern std::unordered_map<std::string, CreatureData> BestiaryDataMap;

void SetupLog();
void LoadDataFromINI();
void PopulateVariantMap();

#endif  // UTILITY_H