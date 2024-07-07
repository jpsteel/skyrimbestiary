#include "Serialization.h"

void RevertCallback(SKSE::SerializationInterface*) {
    spdlog::info("RevertCallback triggered.");
    BestiaryDataMap.clear();
}

void SaveCallback(SKSE::SerializationInterface* intfc) {
    spdlog::info("SaveCallback triggered.");
    intfc->OpenRecord('BSTY', DATA_VERSION);
    Serialize(intfc);
}

void LoadCallback(SKSE::SerializationInterface* intfc) {
    spdlog::info("LoadCallback triggered.");
    uint32_t type, version, length;
    while (intfc->GetNextRecordInfo(type, version, length)) {
        if (type == 'BSTY') {
            spdlog::info("Deserializing record 'BSTY' with version {}", version);
            if (!Deserialize(intfc, version)) {
                spdlog::error("Failed to deserialize BestiaryDataMap");
            }
        } else {
            spdlog::warn("Unknown record type: 0x{:X}", type);
        }
    }
}

void Serialize(SKSE::SerializationInterface* intfc) {
    uint32_t size = static_cast<uint32_t>(BestiaryDataMap.size());
    spdlog::info("Serializing BestiaryDataMap: {} entries", size);
    intfc->WriteRecordData(&size, sizeof(size));

    for (const auto& [creatureName, data] : BestiaryDataMap) {
        uint32_t length = static_cast<uint32_t>(creatureName.length());
        spdlog::debug("Serializing creature: {}", creatureName);
        intfc->WriteRecordData(&length, sizeof(length));
        intfc->WriteRecordData(creatureName.data(), length);
        data.Serialize(intfc);
    }
}

bool Deserialize(SKSE::SerializationInterface* intfc, uint32_t version) {
    (void)version;
    uint32_t size;
    if (!intfc->ReadRecordData(&size, sizeof(size))) {
        spdlog::error("Failed to read size of BestiaryDataMap");
        return false;
    }
    spdlog::info("Deserializing BestiaryDataMap: {} entries", size);
    BestiaryDataMap.clear();

    for (uint32_t i = 0; i < size; ++i) {
        uint32_t length;
        if (!intfc->ReadRecordData(&length, sizeof(length))) {
            spdlog::error("Failed to read length of creature name");
            return false;
        }
        std::string creatureName(length, '\0');
        if (!intfc->ReadRecordData(&creatureName[0], length)) {
            spdlog::error("Failed to read creature name");
            return false;
        }
        CreatureData data;
        if (!data.Deserialize(intfc)) {
            spdlog::error("Failed to deserialize creature data for {}", creatureName);
            return false;
        }
        spdlog::debug("Deserialized creature: {}", creatureName);
        BestiaryDataMap[creatureName] = data;
    }
    return true;
}

void CreatureData::Serialize(SKSE::SerializationInterface* intfc) const {
    intfc->WriteRecordData(kills);
    intfc->WriteRecordData(summons);
    intfc->WriteRecordData(transformations);
}

bool CreatureData::Deserialize(SKSE::SerializationInterface* intfc) {
    return intfc->ReadRecordData(kills) && intfc->ReadRecordData(summons) && intfc->ReadRecordData(transformations);
}