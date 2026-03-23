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
    lastOffsetN{0xFFFFFFFF} 
{}

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
        if (hasRam && ramEnabled && !eram.empty()) 
            return eramBankn[address - 0xA000];
        return 0xFF;
    }
    return 0xFF; 
}

//Currently only MBC1 is supported 
void Cartridge::write(uint16_t address, uint8_t data) {
    if(mbc == MBC::MBC1) {
        if(address >= 0x0000 && address <= 0x1FFF) {
            if(data == 0x0A)
                ramEnabled = true;
            else 
                ramEnabled = false;
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
            if(hasRam && ramEnabled) 
                eramBankn[address - 0xA000] = data;
        }

        updateOffsets();

    }


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
    case 0x03:
        hasBatery = true;
        [[fallthrough]];
    case 0x02:
        hasRam = true;
        [[fallthrough]];
    case 0x01:
        mbc = MBC::MBC1;
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
    uint8_t fullRomBank = (bankReg2 << 5) | bankReg1;
    uint32_t romOffsetN = (fullRomBank % (romSize / 0x4000)) * 0x4000;

    if (!isStreaming) {
        romBankn = romData + romOffsetN;
    } else {
        romBankn = getCachedBank(romOffsetN);
        lastOffsetN = romOffsetN;
    }

    uint32_t romOffset0 = 0;
    if (mbcMode == 1) {
        romOffset0 = ((bankReg2 << 5) % (romSize / 0x4000)) * 0x4000;
    }

    if (!isStreaming) {
        romBank0 = romData + romOffset0;
    } else {
        romBank0 = getCachedBank(romOffset0);
        lastOffset0 = romOffset0;
    }

    if (hasRam && !eram.empty()) {
        uint32_t ramOffset = 0;
        if (mbcMode == 1) {
            ramOffset = (bankReg2 % (eram.size() / 0x2000)) * 0x2000;
        }
        eramBankn = eram.data() + ramOffset;
    }
}