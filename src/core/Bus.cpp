#include "Bus.h"
#include "Timer.h"

Bus::Bus(PPU& p, ICartridge* c, Timer& t, IJoypad& j) : wram{}, hram{}, ppu{p}, cartridge{c}, ie_register{}, io{}, joypad{j}, timer{t} {}

uint8_t Bus::read(uint16_t address) {
    //ROM
    if(address >= 0x0000 && address <= 0x7FFF) {
        return cartridge->read(address);
    } 
    //VRAM
    else if(address >= 0x8000 && address <= 0x9FFF) {
        return ppu.read(address);
    }
    //ERAM
    else if (address >= 0xA000 && address <= 0xBFFF) {
        return cartridge->read(address);
    }
    //WRAM
    else if(address >= 0xC000 && address <= 0xDFFF) {
        return wram[address - 0xC000];
    }
    //Echo
    else if(address >= 0xE000 && address <= 0xFDFF) {
        return wram[address - 0xE000];
    }
    //OAM
    else if(address >= 0xFE00 && address <= 0xFE9F) {
        return ppu.read(address);
    }
    //Joypad
    else if(address == 0xFF00) {
        return joypad.getState();
    }
     //PPU registers
    else if(address >= 0xFF40 && address <= 0xFF4B) {
        return ppu.read(address);
    }
    //Timer
    else if(address >= 0xFF04 && address <= 0xFF07) {
        return timer.read(address);
    }
    //I/O  except timer and ppu reg and joypad
    else if(address >= 0xFF00 && address <= 0xFF7F) {
        if (address == 0xFF0F) {
            return io[0x0F] | 0xE0; // The top 3 bits of IF always read as 1
        }
        return io[address - 0xFF00];
    }
    //HRAM
    else if(address >= 0xFF80 && address <= 0xFFFE) {
        return hram[address - 0xFF80];
    }
    //IE
    else if(address == 0xFFFF) {
        return ie_register;
    }
    //Not used addresses
    else {
        return 0xFF;
    }
}

void Bus::write(uint16_t address, uint8_t data) {
    //ROM
    if(address >= 0x0000 && address <= 0x7FFF) {
        cartridge->write(address,data);
    } 
    //VRAM
    else if(address >= 0x8000 && address <= 0x9FFF) {
        ppu.write(address,data);
    }
    //ERAM
    else if (address >= 0xA000 && address <= 0xBFFF) {
        cartridge->write(address, data);
    }
    //WRAM
    else if(address >= 0xC000 && address <= 0xDFFF) {
        wram[address - 0xC000] = data;
    }
    //Echo
    else if(address >= 0xE000 && address <= 0xFDFF) {
        wram[address - 0xE000] = data;
    }
    //OAM
    else if(address >= 0xFE00 && address <= 0xFE9F) {
        ppu.write(address,data);
    }
    //Joypad
    else if(address == 0xFF00) {
        joypad.setSelector(data);
        return;
    }
    //PPU registers
    else if(address >= 0xFF40 && address <= 0xFF4B) {
        ppu.write(address,data);
        return;
    }
    //Timer
    else if(address >= 0xFF04 && address <= 0xFF07) {
        timer.write(address, data);
        return;
    }
    //I/O except timer and ppu reg and joypad
    else if(address >= 0xFF00 && address <= 0xFF7F) {
        io[address - 0xFF00] = data;
    }
    //HRAM
    else if(address >= 0xFF80 && address <= 0xFFFE) {
        hram[address - 0xFF80] = data;
    }
    //IE
    else if(address == 0xFFFF) {
        ie_register = data;
    }

}

void Bus::requestInterrupt(uint8_t interrupt) {
    io[0x0F] |= (1 << interrupt);
}