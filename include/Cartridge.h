#pragma once
#include <vector>
#include <cstdint>
#include <string>

class Cartridge
{
private:

    enum class MBC : uint8_t { MBC0 = 0, MBC1 = 1, MBC2 = 2, MBC3 = 3, MBC4 = 4, MBC5 = 5};

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
    uint8_t bankReg1; // Holds the 5 bits written from 0x2000-0x3FFF
    uint8_t bankReg2; // Holds the 2 bits written from 0x4000–0x5FFF
    MBC mbc;

    // Write changes only the bank registers
    void updateOffsets();
public:
    Cartridge();
    bool load_rom(const uint8_t* data, size_t size);    
    uint8_t read(uint16_t address);
    void write(uint16_t address, uint8_t data);
};

