#include "Cartridge.h"

Cartridge::Cartridge() : 
    isStreaming{false},
    romFile{},
    romCache{},
    accessCounter{0},
    romData{nullptr},
    romSize{0}, 
    romBank0{nullptr}, 
    romBankn{nullptr}, 
    eram{}, 
    eramBankn{nullptr}, 
    hasRam{false}, 
    ramEnabled{false}, 
    hasBatery{false}, 
    mbcMode{0}, 
    bankReg1{1}, 
    bankReg2{0}, 
    mbc{MBC::MBC0},
    lastOffset0{0xFFFFFFFF},
    lastOffsetN{0xFFFFFFFF},
    rtcRegs{},
    rtcLatchedRegs{},
    rtcLatchedValue{0xFF}
{}

uint8_t Cartridge::read(uint16_t address) {
    //bank0
    if(address >= 0x0000 && address <= 0x3FFF) {
        if (isStreaming) return getCachedBank(lastOffset0)[address];
        return romBank0[address];
    } 
    //bankn
    else if(address >= 0x4000 && address <= 0x7FFF) {
        if (isStreaming) return getCachedBank(lastOffsetN)[address - 0x4000];
        return romBankn[address - 0x4000];
    }
    //ERAM
    else if (address >= 0xA000 && address <= 0xBFFF) {
        if(mbc == MBC::MBC3) {
            if (ramEnabled) {
                if(bankReg2 >= 0x08 && bankReg2 <= 0x0C)
                    return rtcLatchedRegs[bankReg2 - 0x08];
                else return eramBankn[address - 0xA000];
            }
        } else if (hasRam && ramEnabled && !eram.empty()) 
            return eramBankn[address - 0xA000];
        return 0xFF;
    }
    return 0xFF; 
}

//Currently only MBC1, 3 and 5 are supported 
void Cartridge::write(uint16_t address, uint8_t data) {

    if(mbc == MBC::MBC0)
        return;

    if(mbc == MBC::MBC1) {
       if(address >= 0x0000 && address <= 0x1FFF) {
            ramEnabled = ((data & 0x0F) == 0x0A);
        }

        if(address >= 0x2000 && address <= 0x3FFF) {
            bankReg1 = data & 0x1F;
            if(bankReg1 == 0) bankReg1 = 1;
        } else if(address >= 0x4000 && address <= 0x5FFF) {
            bankReg2 = data & 0x03;
        }

        if(address >= 0x6000 && address <= 0x7FFF) {
            mbcMode = data & 0x01;
        }

        if(address >= 0xA000 && address <= 0xBFFF) {
            if(hasRam && ramEnabled && eramBankn) 
                eramBankn[address - 0xA000] = data;
        }
    } else if(mbc == MBC::MBC3) {
       if(address >= 0x0000 && address <= 0x1FFF) {
            ramEnabled = ((data & 0x0F) == 0x0A);
        }

        if(address >= 0x2000 && address <= 0x3FFF) {
            bankReg1 = data & 0x7F;
            if(bankReg1 == 0) bankReg1 = 1;
        } else if(address >= 0x4000 && address <= 0x5FFF) {
            bankReg2 = data & 0x0F;
        }

        if(address >= 0x6000 && address <= 0x7FFF) {
            if (rtcLatchedValue == 0x00 && data == 0x01) {
                    for(int i = 0; i < 5; i++) {
                        rtcLatchedRegs[i] = rtcRegs[i];
                    }
            }
            rtcLatchedValue = data;
        }

        if(address >= 0xA000 && address <= 0xBFFF) {
            if(ramEnabled) {
                if(bankReg2 <= 0x03) {
                    if(hasRam && !eram.empty()) 
                        eramBankn[address - 0xA000] = data;
                } else if(bankReg2 >= 0x08 && bankReg2 <= 0x0C) {
                    rtcRegs[bankReg2 - 0x08] = data;
                }
            }
        }
    } else if(mbc == MBC::MBC5) {
        if(address >= 0x0000 && address <= 0x1FFF) {
            ramEnabled = ((data & 0x0F) == 0x0A);
        }

        if(address >= 0x2000 && address <= 0x2FFF) {
            bankReg1 = data;
        } else if(address >= 0x3000 && address <= 0x3FFF) {
            bankReg2 = data & 0x01;
        }

        if(address >= 0x4000 && address <= 0x5FFF) {
            mbcMode = data & 0x0F;
        }

        if(address >= 0xA000 && address <= 0xBFFF) {
            if(hasRam && ramEnabled && eramBankn) 
                eramBankn[address - 0xA000] = data;
        }
    }

    updateOffsets();
}
bool Cartridge::load_rom(const uint8_t* data, size_t size, bool stream, const char* filename) {
    // A Gameboy rom must be 32 KiB or larger depending on mbc
    if(size < 0x8000) {
        return false;
    } 
    
    romSize = size;

    if(!stream) {
        isStreaming = false;
        romData = data;
    } else if(filename != nullptr) {
        isStreaming = true;
        if (romFile) romFile.close();
        romFile = SD.open(filename);
        if (!romFile) return false;
        romData = nullptr;
    } else {
        return false;
    }

    accessCounter = 0;
    for(int i=0; i < MAX_CACHED_BANKS; i++) {
        romCache[i].offset = 0xFFFFFFFF;
        romCache[i].lastUsed = 0;
    }

    // Reset mode
    mbcMode = 0;

    // MBC1
    uint8_t mbcType;
    uint8_t ramSizeCode;
    if(isStreaming) {
        uint8_t header[0x150]; 
        romFile.seek(0);
        romFile.read(header, 0x150);
        mbcType = header[0x147];

        ramSizeCode = header[0x149];
        romBank0 = getCachedBank(0);
        romBankn = getCachedBank(0x4000);
    } else {
        // Initialize banks
        romBank0 = romData;
        romBankn = romData + 0x4000;
        mbcType = romData[0x147];

        ramSizeCode = romData[0x149];

    }

    switch (mbcType)
    {
    //MBC1 
    case 0x03:
        hasBatery = true;
        [[fallthrough]];
    case 0x02:
        hasRam = true;
        [[fallthrough]];
    case 0x01:
        mbc = MBC::MBC1;
        break;

    // MBC3
    case 0x13: // MBC3 + RAM + BATTERY
    case 0x10: // MBC3 + TIMER + BATTERY
    case 0x0F: // MBC3 + TIMER + BATTERY
        hasBatery = true;
        [[fallthrough]];
    case 0x12: // MBC3 + RAM
        hasRam = true;
        [[fallthrough]];
    case 0x11: // MBC3 (Plain)
        mbc = MBC::MBC3;
        break;

    //MBC5
    case 0x1E: // MBC5 + RUMBLE + RAM + BATTERY
    case 0x1B: // MBC5 + RAM + BATTERY
        hasBatery = true;
        [[fallthrough]];
    case 0x1D: // MBC5 + RUMBLE + RAM
    case 0x1A: // MBC5 + RAM
        hasRam = true;
        [[fallthrough]];
    case 0x1C: // MBC5 + RUMBLE
    case 0x19: // MBC5 (Plain)
        mbc = MBC::MBC5;
        break;
    default:
        mbc = MBC::MBC0;
        break;
    }

    uint32_t ramSize = 0;
    switch(ramSizeCode) {
        case 0x02: ramSize = 8192; break;    // 8KB
        case 0x03: ramSize = 32768; break;   // 32KB
        case 0x04: ramSize = 131072; break;  // 128KB
        // ...
    }
        if (ramSize > 0) {
          eram.resize(ramSize, 0xFF); 
    }

    
    lastOffsetN = 0xFFFFFFFF;
    lastOffset0 = 0xFFFFFFFF;
    bankReg1 = 1;
    bankReg2 = 0;
    updateOffsets();
    return true;
}
const uint8_t* Cartridge::getCachedBank(uint32_t offset) {
    //Check if already in cache
    for (int i = 0; i < MAX_CACHED_BANKS; i++) {
        if (romCache[i].offset == offset) {
            romCache[i].lastUsed = ++accessCounter;
            return romCache[i].data;
        }
    }

    //LRU: Find the least recently used slot
    int lruIdx = 0;
    for (int i = 1; i < MAX_CACHED_BANKS; i++) {
        if (romCache[i].lastUsed < romCache[lruIdx].lastUsed) lruIdx = i;
    }

    romFile.seek(offset);
    romFile.read(romCache[lruIdx].data, 0x4000);

    romCache[lruIdx].offset = offset;
    romCache[lruIdx].lastUsed = ++accessCounter;
    return romCache[lruIdx].data;
}

void Cartridge::updateOffsets() {
    uint32_t romOffsetN = 0x4000;
    uint32_t ramOffset = 0;
    uint32_t romOffset0 = 0;
    switch (mbc)
    {
    case MBC::MBC0:
    {
        if (!isStreaming) {
            romBank0 = romData;
            romBankn = romData + 0x4000;
        } else {
            // Force lock to the first two banks
            if (lastOffset0 != 0) {
                romBank0 = getCachedBank(0);
                lastOffset0 = 0;
            }
            if (lastOffsetN != 0x4000) {
                romBankn = getCachedBank(0x4000);
                lastOffsetN = 0x4000;
            }
        }
        break;
    }
    case MBC::MBC1:
    {   
        uint8_t fullRomBank = (bankReg2 << 5) | bankReg1;
        romOffsetN = (fullRomBank % (romSize / 0x4000)) * 0x4000;

        if (!isStreaming) {
            romBankn = romData + romOffsetN;
        } else if (romOffsetN != lastOffsetN){
            romBankn = getCachedBank(romOffsetN);
            lastOffsetN = romOffsetN;
        }

        if (mbcMode == 1) {
            romOffset0 = ((bankReg2 << 5) % (romSize / 0x4000)) * 0x4000;
        }

        if (!isStreaming) {
            romBank0 = romData + romOffset0;
        } else if (romOffset0 != lastOffset0){
            romBank0 = getCachedBank(romOffset0);
            lastOffset0 = romOffset0;
        }

        if (hasRam && !eram.empty()) {
            if (mbcMode == 1) {
                ramOffset = (bankReg2 % (eram.size() / 0x2000)) * 0x2000;
            }
            eramBankn = eram.data() + ramOffset;
        }
        break;

    } case MBC::MBC3: 
    {
        uint16_t fullRomBank = bankReg1;
        romOffsetN = (fullRomBank % (romSize / 0x4000)) * 0x4000;

        if (!isStreaming) {
            romBankn = romData + romOffsetN;
            romBank0 = romData + romOffset0;
        } else  {
            if (romOffset0 != lastOffset0) {
                romBank0 = getCachedBank(romOffset0);
                lastOffset0 = romOffset0;
            }
            if (romOffsetN != lastOffsetN) {
                romBankn = getCachedBank(romOffsetN);
                lastOffsetN = romOffsetN;
            }
        }


        if (hasRam && !eram.empty()) {
            if (bankReg2 <= 0x03) { // Only banks 0x00-0x03 map to actual RAM
                ramOffset = (bankReg2 % (eram.size() / 0x2000)) * 0x2000;
                eramBankn = eram.data() + ramOffset;
            }
        }
        break;

    } case MBC::MBC5:
    {
        uint16_t fullRomBank = (static_cast<uint16_t>(bankReg2 & 0x01) << 8) | bankReg1;
        romOffsetN = (fullRomBank % (romSize / 0x4000)) * 0x4000;

        if (!isStreaming) {
            romBankn = romData + romOffsetN;
            romBank0 = romData + romOffset0;
        } else  {
            if (romOffset0 != lastOffset0) {
                romBank0 = getCachedBank(romOffset0);
                lastOffset0 = romOffset0;
            }
            if (romOffsetN != lastOffsetN) {
                romBankn = getCachedBank(romOffsetN);
                lastOffsetN = romOffsetN;
            }
        }


        if (hasRam && !eram.empty()) {
            ramOffset = (mbcMode % (eram.size() / 0x2000)) * 0x2000;
            eramBankn = eram.data() + ramOffset;
        }
        break;

    }
    default:
        break;
    }


}
void Cartridge::addSeconds(uint32_t seconds) {
    // Don't tick if the HALT bit is set (Bit 6 of Day High)
    if (rtcRegs[4] & 0x40) return;

    uint32_t s = rtcRegs[0] + seconds;
    rtcRegs[0] = s % 60;
    
    uint32_t m = rtcRegs[1] + (s / 60);
    rtcRegs[1] = m % 60;
    
    uint32_t h = rtcRegs[2] + (m / 60);
    rtcRegs[2] = h % 24;
    
    // Day counter is 9 bits total (Day Low register + 1 bit in Day High)
    uint16_t d = (rtcRegs[3] | ((rtcRegs[4] & 0x01) << 8)) + (h / 24);
    uint16_t finalDays = d % 512; 
    rtcRegs[3] = finalDays & 0xFF;
    rtcRegs[4] = (rtcRegs[4] & 0xFE) | ((finalDays >> 8) & 0x01);

    if (d > 511) {
        rtcRegs[4] |= 0x80; // Carry bit remains set until manually cleared
    }
}