#pragma once
#include <cstdint>

class PPU
{
private:
    uint8_t vram[0x2000];
    uint8_t oam[0xA0];
public:
    PPU();
    
    uint8_t read(uint16_t address);
    void write(uint16_t address, uint8_t data);
};

