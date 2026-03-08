#include "Cartridge.h"

Cartridge::Cartridge() : rom{}, romBank0{nullptr}, romBankn{nullptr}, eram{}, eramBankn{nullptr} {}

uint8_t Cartridge::read(uint16_t address) {
    //bank0
    if(address >= 0x0000 && address <= 0x3FFF) {
        return romBank0[address];
    } 
    //bankn
    else if(address >= 0x4000 && address <= 0x7FFF) {
        return romBankn[address - 0x4000];
    }
    //ERAM
    else if (address >= 0xA000 && address <= 0xBFFF) {
        if(eramBankn != nullptr)
            return eramBankn[address - 0xA000];
        return 0xFF;
    }
    return 0xFF; 
}

//Currently only No MBC is supported
void Cartridge::write(uint16_t address, uint8_t data) {

}

bool Cartridge::load_rom(const std::vector<uint8_t>& rom_data) {
    //A Gameboy rom must be than 32 KiB or larger depending on mbc
    if(rom_data.size() < 0x8000) {
        return false;
    } 
    rom = rom_data;

    //Initialize banks
    romBank0 = rom.data();
    romBankn = rom.data() + 0x4000;

    //TODO add mbc support
    return true;
}