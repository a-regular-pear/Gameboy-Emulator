#include "Cartridge.h"

Cartridge::Cartridge() : romData{nullptr},romSize{}, romBank0{nullptr}, romBankn{nullptr}, eram{}, eramBankn{nullptr}, 
                         hasRam{false}, ramEnabled{false}, hasBatery{false}, mbcMode{0}, 
                         bankReg1{1}, bankReg2{0}, mbc{MBC::MBC0} {}
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
        if (hasRam && ramEnabled) 
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
bool Cartridge::load_rom(const uint8_t* data, size_t size) {
    // A Gameboy rom must be 32 KiB or larger depending on mbc
    if(size < 0x8000) {
        return false;
    } 
    
    romData = data;
    romSize = size;

    // Initialize banks
    romBank0 = romData;
    romBankn = romData + 0x4000;

    // Reset mode
    mbcMode = 0;

    // MBC1
    uint8_t mbcType = romData[0x147];

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

    uint8_t ramSizeCode = romData[0x149];
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
    bankReg1 = 1;
    bankReg2 = 0;
    updateOffsets();
    return true;
}

void Cartridge::updateOffsets() {
    uint8_t fullRomBank = (bankReg2 << 5) | bankReg1;
    // the multiplication with 0x4000 is because everybank is 16Kib
    uint32_t romOffsetN = fullRomBank * 0x4000;
    
    if (romOffsetN < romSize) {
        romBankn = romData + romOffsetN;
    }

    // romBank0 is only affected in mode 1
    if(mbcMode == 0) {
        romBank0 = romData;
    } else if (mbcMode == 1) {
        uint32_t romOffset0 = (bankReg2 << 5) * 0x4000;
        if (romOffset0 < romSize) {
            romBank0 = romData + romOffset0;
        }
    }

    if(hasRam && !eram.empty()) {
        if (mbcMode == 0) {
            eramBankn = eram.data();
        } else if(mbcMode == 1) {
            uint32_t ramOffset = bankReg2 * 0x2000;
            if (ramOffset < eram.size()) {
                eramBankn = eram.data() + ramOffset;
            }
        }
    }
}