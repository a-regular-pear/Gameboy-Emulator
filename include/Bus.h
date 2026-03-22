#pragma once
#include <cstdint>
#include "Cartridge.h"
#include "PPU.h"
#include "Timer.h"
#include "Joypad.h"

class Timer; // forward declaration

class Bus {
private:
    uint8_t wram[0x2000]; // 8KB Work RAM
    uint8_t hram[0x7F];   // High RAM
    
    PPU& ppu;

    // cartridge is a pointer so it can be chnaged wthout need to recreate bus
    Cartridge* cartridge;
    
    // Interrupt
    uint8_t ie_register;

    // temporary IO registers
    uint8_t io[0x80];
    Joypad& joypad;
    Timer& timer;
public:
    Bus(PPU& p, Cartridge* c,Timer& timer, Joypad& joypad);    
    uint8_t read(uint16_t address);
    void write(uint16_t address, uint8_t data);
    // Enables specified IF bit (for example for interrupt 0 it turns on the LSB)
    void requestInterrupt(uint8_t interrupt);
};