#include "PPU.h"

PPU::PPU() : vram{}, oam{} {}


uint8_t PPU::read(uint16_t address) {
    if(address >= 0x8000 && address <= 0x9FFF) 
    {
        return vram[address - 0x8000];
    }
    else if(address >= 0xFE00 && address <= 0xFE9F) 
    {
        return oam[address - 0xFE00];
    }
}

void PPU::write(uint16_t address,uint8_t data) {
    if(address >= 0x8000 && address <= 0x9FFF) 
    {
        vram[address - 0x8000] = data;
    }
    else if(address >= 0xFE00 && address <= 0xFE9F) 
    {
        oam[address - 0xFE00] = data;
    }
}