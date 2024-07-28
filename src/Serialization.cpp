#include "Serialization.h"

void RevertCallback(SKSE::SerializationInterface*) {
    logger::info("RevertCallback triggered.");
    BestiaryDataMap.clear();
    lastEntry.clear();
    hintShown = false;
}

void SaveCallback(SKSE::SerializationInterface* intfc) {
    logger::info("SaveCallback triggered.");
    intfc->OpenRecord('BSTY', DATA_VERSION);
    Serialize(intfc);

    intfc->OpenRecord('LAST', DATA_VERSION);  // New record for lastEntry
    uint32_t length = static_cast<uint32_t>(lastEntry.size());
    intfc->WriteRecordData(&length, sizeof(length));
    intfc->WriteRecordData(lastEntry.data(), length);

    intfc->OpenRecord('HSHN', DATA_VERSION);
    intfc->WriteRecordData(&hintShown, sizeof(hintShown));
}

void LoadCallback(SKSE::SerializationInterface* intfc) {
    logger::info("LoadCallback triggered.");
    uint32_t type, version, length;
    while (intfc->GetNextRecordInfo(type, version, length)) {
        if (type == 'BSTY') {
            logger::info("Deserializing record 'BSTY' with version {}", version);
            if (!Deserialize(intfc, version)) {
                logger::error("Failed to deserialize BestiaryDataMap");
            }
        } else if (type == 'LAST') {
            logger::info("Deserializing record 'LAST' with version {}", version);
            if (!intfc->ReadRecordData(&length, sizeof(length))) {
                logger::error("Failed to read length of lastEntry");
                continue;
            }
            lastEntry.resize(length);
            if (!intfc->ReadRecordData(lastEntry.data(), length)) {
                logger::error("Failed to read lastEntry");
            }
        } else if (type == 'HSHN') {
            logger::info("Deserializing record 'HSHN' with version {}", version);
            if (length == sizeof(hintShown)) {
                intfc->ReadRecordData(&hintShown, sizeof(hintShown));
            } else {
                logger::error("Incorrect data length for hintShown");
            }
        } else {
            logger::warn("Unknown record type: 0x{:X}", type);
        }
    }
}

void Serialize(SKSE::SerializationInterface* intfc) {
    uint32_t size = static_cast<uint32_t>(BestiaryDataMap.size());
    logger::info("Serializing BestiaryDataMap: {} entries", size);
    intfc->WriteRecordData(&size, sizeof(size));

    for (const auto& [creatureName, data] : BestiaryDataMap) {
        uint32_t length = static_cast<uint32_t>(creatureName.length());
        logger::debug("Serializing creature: {}", creatureName);
        intfc->WriteRecordData(&length, sizeof(length));
        intfc->WriteRecordData(creatureName.data(), length);
        data.Serialize(intfc);
    }
}

bool Deserialize(SKSE::SerializationInterface* intfc, uint32_t version) {
    (void)version;
    uint32_t size;
    if (!intfc->ReadRecordData(&size, sizeof(size))) {
        logger::error("Failed to read size of BestiaryDataMap");
        return false;
    }
    logger::info("Deserializing BestiaryDataMap: {} entries", size);
    BestiaryDataMap.clear();

    for (uint32_t i = 0; i < size; ++i) {
        uint32_t length;
        if (!intfc->ReadRecordData(&length, sizeof(length))) {
            logger::error("Failed to read length of creature name");
            return false;
        }
        std::string creatureName(length, '\0');
        if (!intfc->ReadRecordData(&creatureName[0], length)) {
            logger::error("Failed to read creature name");
            return false;
        }
        CreatureData data;
        if (!data.Deserialize(intfc)) {
            logger::error("Failed to deserialize creature data for {}", creatureName);
            return false;
        }
        logger::debug("Deserialized creature: {}", creatureName);
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