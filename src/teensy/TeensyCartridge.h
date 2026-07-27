#pragma once
#include "ICartridge.h"
#include <SD.h>
#include <TimeLib.h>

#define MAX_CACHED_BANKS 15

class TeensyCartridge : public ICartridge
{
private:
    struct CachedBank {
        uint32_t offset = 0xFFFFFFFF;
        uint32_t lastUsed = 0;
        uint8_t data[0x4000];
    };

    File romFile;
    CachedBank romCache[MAX_CACHED_BANKS];
    uint32_t accessCounter = 0;

protected:
    const uint8_t* getRomBank(uint32_t offset) override;
    bool openFileStream(const char* filename) override;
    void readStreamHeader(uint8_t* header) override;
    uint32_t getPlatformTime() override;

public:
    void save() override;
    void load() override;
};