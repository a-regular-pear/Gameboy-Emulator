#pragma once
#include <cstdint>

class PPU
{
private:

    //This is a pointer and not a reference to avoid issues of circular dependency
    Bus* bus;

    uint8_t vram[0x2000];
    uint8_t oam[0xA0];

    uint8_t dotLineCounter;
    uint8_t lcdc;
    uint8_t stat;
    uint8_t scy, scx;
    uint8_t ly;
    uint8_t lyc;
    uint8_t bgp, obp0, obp1;
    uint8_t wy, wx;

    bool lastSignal;
    void updateMode(uint mode);
    void PPU::compareLYC()
public:
    PPU(Bus* bus);
    
    uint8_t read(uint16_t address);
    void write(uint16_t address, uint8_t data);

    void step(int cycles);
};

