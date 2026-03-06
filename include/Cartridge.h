#pragma once
#include <vector>
#include <cstdint>
#include <string>

class Cartridge
{
private:
    std::vector<uint8_t> rom;
    uint8_t* romBank0;
    uint8_t* romBankn;

    std::vector<uint8_t> eram;
    uint8_t* eramBankn;

public:
    Cartridge();
    void load_rom(const std::string& rom);
    
    uint8_t read(uint16_t address);
    void write(uint16_t address, uint8_t data);
};

