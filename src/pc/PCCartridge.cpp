#include "PCCartridge.h"
#include <sys/stat.h>

bool PCCartridge::openFileStream(const char* filename) {
    if (romFile.is_open()) romFile.close();
    
    romFile.open(filename, std::ios::binary);
    
    accessCounter = 0;
    for(int i=0; i < MAX_PC_CACHED_BANKS; i++) {
        romCache[i].offset = 0xFFFFFFFF;
        romCache[i].lastUsed = 0;
    }
    
    return romFile.is_open();
}

void PCCartridge::readStreamHeader(uint8_t* header) {
    if (romFile.is_open()) {
        romFile.seekg(0, std::ios::beg);
        romFile.read(reinterpret_cast<char*>(header), 0x150);
    }
}

const uint8_t* PCCartridge::getRomBank(uint32_t offset) {
    for (int i = 0; i < MAX_PC_CACHED_BANKS; i++) {
        if (romCache[i].offset == offset) {
            romCache[i].lastUsed = ++accessCounter;
            return romCache[i].data;
        }
    }

    int lruIdx = 0;
    for (int i = 1; i < MAX_PC_CACHED_BANKS; i++) {
        if (romCache[i].lastUsed < romCache[lruIdx].lastUsed) lruIdx = i;
    }

    if (romFile.is_open()) {
        romFile.seekg(offset, std::ios::beg);
        romFile.read(reinterpret_cast<char*>(romCache[lruIdx].data), 0x4000);
    }

    romCache[lruIdx].offset = offset;
    romCache[lruIdx].lastUsed = ++accessCounter;
    return romCache[lruIdx].data;
}

uint32_t PCCartridge::getPlatformTime() {
    auto now = std::chrono::system_clock::now();
    return static_cast<uint32_t>(std::chrono::system_clock::to_time_t(now));
}

void PCCartridge::save() {
    if (currentSaveName.empty()) return;

    std::ofstream file(currentSaveName, std::ios::binary | std::ios::trunc);
    if (!file.is_open()) return;

    if (!eram.empty()) {
        file.write(reinterpret_cast<const char*>(eram.data()), eram.size());
    }

    if (mbc == MBC::MBC3) {
        file.write(reinterpret_cast<const char*>(rtcRegs), 5);
        uint32_t timestamp = getPlatformTime();
        file.write(reinterpret_cast<const char*>(&timestamp), 4);
    }
}

void PCCartridge::load() {
    if (currentSaveName.empty()) return;
    
    std::ifstream file(currentSaveName, std::ios::binary);
    if (!file.is_open()) return;

    if (!eram.empty()) {
        file.read(reinterpret_cast<char*>(eram.data()), eram.size());
    }

    file.seekg(0, std::ios::end);
    std::streamsize size = file.tellg();
    
    if (mbc == MBC::MBC3 && size >= (eram.size() + 9)) {
        file.seekg(eram.size(), std::ios::beg);
        file.read(reinterpret_cast<char*>(rtcRegs), 5);
        
        uint32_t savedTimestamp;
        file.read(reinterpret_cast<char*>(&savedTimestamp), 4);
        
        uint32_t currentTimestamp = getPlatformTime();
        if (currentTimestamp > savedTimestamp) {
            uint32_t elapsedSeconds = currentTimestamp - savedTimestamp;
            addSeconds(elapsedSeconds);
        }
    }
    
    updateOffsets();
}