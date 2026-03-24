#pragma once
#include <vector>
#include <cstdint>
#include <string>
#include <SD.h>
#define MAX_CACHED_BANKS 15

class Cartridge
{
private:

    struct CachedBank {
        uint32_t offset = 0xFFFFFFFF; 
        uint32_t lastUsed = 0;        
        uint8_t data[0x4000];         
    };

    enum class MBC : uint8_t { MBC0 = 0, MBC1 = 1, MBC2 = 2, MBC3 = 3, MBC4 = 4, MBC5 = 5};

    bool isStreaming;
    File romFile;
    CachedBank romCache[MAX_CACHED_BANKS];
    uint32_t accessCounter = 0;

    const uint8_t* romData; 
    size_t romSize;         
    const uint8_t* romBank0;
    const uint8_t* romBankn;

    std::vector<uint8_t> eram;
    uint8_t* eramBankn;
    bool hasRam;
    bool ramEnabled;
    bool hasBatery;
    uint8_t mbcMode;
    uint8_t bankReg1; 
    uint8_t bankReg2; 
    MBC mbc;
    uint32_t lastOffset0;
    uint32_t lastOffsetN;
    const uint8_t* getCachedBank(uint32_t offset);

    // Write changes only the bank registers
    void updateOffsets();
public:
    Cartridge();
    bool load_rom(const uint8_t* data, size_t size, bool stream = false, const char* filename = "");    
    uint8_t read(uint16_t address);
    void write(uint16_t address, uint8_t data);
};

