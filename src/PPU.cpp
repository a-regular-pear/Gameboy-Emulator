#include "PPU.h"

PPU::PPU(Bus* b) : 
    bus{b}, 
    vram{}, 
    oam{}, 
    dotLineCounter{0}, 
    lcdc{0x91}, 
    stat{0x85}, 
    scy{0}, scx{0}, 
    ly{0}, lyc{0}, 
    bgp{0xFC}, obp0{0xFF}, obp1{0xFF}, 
    wy{0}, wx{0}, 
    lastSignal{false} 
{}


uint8_t PPU::read(uint16_t address) {
    if(address >= 0x8000 && address <= 0x9FFF) 
    {
        return vram[address - 0x8000];
    }
    else if(address >= 0xFE00 && address <= 0xFE9F) 
    {
        return oam[address - 0xFE00];
    } else {
        //DMA ($FF46) is not an actual register therefor it cant be read
        switch (address)
        {
        case 0xFF40: return lcdc;
        case 0xFF41: return stat;
        case 0xFF42: return scy;
        case 0xFF43: return scx;
        case 0xFF44: return ly;
        case 0xFF45: return lyc;
        case 0xFF47: return bgp;
        case 0xFF48: return obp0;
        case 0xFF49: return obp1;
        case 0xFF4A: return wy;
        case 0xFF4B: return wx;        
        default: return 0xFF;
        }
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
    } else {
        switch (address)
        {
        //LY is read only
        case 0xFF40: {
            lcdc = data; 
            //Check if display is off
            if(!(lcdc & (1 << 7))) {
                stat = 0;
                dotLineCounter = 0;
                ly = 0;
            }
            break;
        }
        case 0xFF41: stat = (stat & 0x07) | (data & 0xF8) | 0x80; break;
        case 0xFF42: scy = data; break;
        case 0xFF43: scx = data; break;
        case 0xFF45: lyc = data; compareLYC(); break;
        case 0xFF46: 
        {
            uint16_t source = data << 8;
            for(int i = 0; i < 160; i++) {
                oam[i] = bus->read(source + i);
            }
            break;
        }
        case 0xFF47: bgp = data; break;
        case 0xFF48: obp0 = data; break;
        case 0xFF49: obp1 = data; break;
        case 0xFF4A: wy = data; break;
        case 0xFF4B: wx = data; break;        
        default: break;
        }
    }
}

void PPU::step(int cycles) {
    //Check if display is off
    if(!(lcdc & (1 << 7)))  return;

    dotLineCounter += cycles;
    if(dotLineCounter >= 456) {
        dotLineCounter -= 456;
        ly++

        compareLYC();
        if(ly == 144) {
            bus->requestInterrupt(0);
        } else if(ly > 153) {
            ly = 0;
            return;
        }
    }

    //TODO Make mode 3 and 0 non static
    if(ly >= 144) {
        updateMode(1);
    } else if(dotLineCounter < 80) {
        updateMode(2);
    } else if(dotLineCounter < 252) {
        updateMode(3);
    } else if(dotLineCounter < 456) {
        updateMode(0);
    }

}

void PPU::updateMode(uint8_t mode) {
    //Check if mode has changed
    uint8_t currentMode = stat & 0x03;
    if(currentMode == mode) return;

    stat = (stat & 0xFC) | (mode & 0x03);

    updateInterrupts();
}

void PPU::compareLYC() {
    if (ly == lyc) {
        stat |= (1 << 2);     
    } else {
        stat &= ~(1 << 2);    
    } 
    updateInterrupts();

}

void PPU::updateInterrupts() {
    uint8_t mode = stat & 0x03;
    bool lycInterrupt = (stat & (1 << 6)) && (stat & (1 << 2));
    bool m0Interrupt = (stat & (1 << 3)) && (mode == 0);
    bool m1Interrupt = (stat & (1 << 4)) && (mode == 1);
    bool m2Interrupt = (stat & (1 << 5)) && (mode == 2);
    bool currentSignal = lycInterrupt || m0Interrupt || m1Interrupt || m2Interrupt;
    if(currentSignal && ! lastSignal) bus->requestInterrupt(1);
    lastSignal = currentSignal;
}