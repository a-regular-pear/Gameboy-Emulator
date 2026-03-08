#pragma once
#include <vector>
#include <cstdint>
#include <string>

class Cartridge
{
private:
    std::vector<uint8_t> rom;
    const uint8_t* romBank0;
    const uint8_t* romBankn;

    std::vector<uint8_t> eram;
    const uint8_t* eramBankn;

public:
    Cartridge();
    bool load_rom(const std::vector<uint8_t>& rom_data);
    
    uint8_t read(uint16_t address);
    void write(uint16_t address, uint8_t data);
};

