#include "TeensyCartridge.h"

bool TeensyCartridge::openFileStream(const char* filename) {
    if (romFile) romFile.close();
    romFile = SD.open(filename);
    
    accessCounter = 0;
    for(int i=0; i < MAX_CACHED_BANKS; i++) {
        romCache[i].offset = 0xFFFFFFFF;
        romCache[i].lastUsed = 0;
    }
    
    return static_cast<bool>(romFile);
}

void TeensyCartridge::readStreamHeader(uint8_t* header) {
    if (romFile) {
        romFile.seek(0);
        romFile.read(header, 0x150);
    }
}

const uint8_t* TeensyCartridge::getRomBank(uint32_t offset) {
    for (int i = 0; i < MAX_CACHED_BANKS; i++) {
        if (romCache[i].offset == offset) {
            romCache[i].lastUsed = ++accessCounter;
            return romCache[i].data;
        }
    }

    int lruIdx = 0;
    for (int i = 1; i < MAX_CACHED_BANKS; i++) {
        if (romCache[i].lastUsed < romCache[lruIdx].lastUsed) lruIdx = i;
    }

    if (romFile) {
        romFile.seek(offset);
        romFile.read(romCache[lruIdx].data, 0x4000);
    }

    romCache[lruIdx].offset = offset;
    romCache[lruIdx].lastUsed = ++accessCounter;
    return romCache[lruIdx].data;
}

uint32_t TeensyCartridge::getPlatformTime() {
    return now();
}

void TeensyCartridge::save() {
    if (currentSaveName.empty()) return;

    SD.remove(currentSaveName.c_str());

    File file = SD.open(currentSaveName.c_str(), FILE_WRITE);
    if (!file) {
        Serial.println("CRITICAL: SD.open failed! Is the card locked or removed?");
        return;
    }

    if (!eram.empty()) {
        file.write(eram.data(), eram.size());
    }

    if (mbc == MBC::MBC3) {
        file.write(rtcRegs, 5);
        uint32_t timestamp = getPlatformTime();
        file.write((uint8_t*)&timestamp, 4);
    }

    file.flush();
    file.close();
}

void TeensyCartridge::load() {
    if (!SD.exists(currentSaveName.c_str())) return;
    
    File file = SD.open(currentSaveName.c_str(), FILE_READ);
    if (!file) return;

    if (!eram.empty()) {
        file.read(eram.data(), eram.size());
    }

    if (mbc == MBC::MBC3 && file.available() >= 9) {
        file.read(rtcRegs, 5);
        
        uint32_t savedTimestamp;
        file.read((uint8_t*)&savedTimestamp, 4);
        
        uint32_t currentTimestamp = getPlatformTime();
        if (currentTimestamp > savedTimestamp) {
            uint32_t elapsedSeconds = currentTimestamp - savedTimestamp;
            addSeconds(elapsedSeconds);
        }
    }
    file.close();
    updateOffsets();
}