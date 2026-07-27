#include "ICartridge.h"
#include <cstring>

ICartridge::ICartridge() :
    isStreaming{false},
    romData{nullptr},
    romSize{0},
    romBank0{nullptr},
    romBankn{nullptr},
    eram{},
    eramBankn{nullptr},
    hasRam{false},
    ramEnabled{false},
    hasBattery{false},
    mbcMode{0},
    bankReg1{1},
    bankReg2{0},
    mbc{MBC::MBC0},
    lastOffset0{0xFFFFFFFF},
    lastOffsetN{0xFFFFFFFF},
    rtcRegs{},
    rtcLatchedRegs{},
    rtcLatchedValue{0xFF},
    isDirty{true},
    forceSave{false}
{}

uint8_t ICartridge::read(uint16_t address) {
    if(address >= 0x0000 && address <= 0x3FFF) {
        if (isStreaming) return getRomBank(lastOffset0)[address];
        return romBank0[address];
    }
    else if(address >= 0x4000 && address <= 0x7FFF) {
        if (isStreaming) return getRomBank(lastOffsetN)[address - 0x4000];
        return romBankn[address - 0x4000];
    }
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

void ICartridge::write(uint16_t address, uint8_t data) {
    if(mbc == MBC::MBC0) return;

    if(mbc == MBC::MBC1) {
       if(address >= 0x0000 && address <= 0x1FFF) {
            bool newRamEnabled = ((data & 0x0F) == 0x0A);
            if (ramEnabled && !newRamEnabled && isDirty) {
                forceSave = true;
            }
            ramEnabled = newRamEnabled;
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
            if(hasRam && ramEnabled && eramBankn) {
                if (eramBankn[address - 0xA000] != data) {
                    eramBankn[address - 0xA000] = data;
                    isDirty = true;
                }
            }
        }
    } else if(mbc == MBC::MBC3) {
       if(address >= 0x0000 && address <= 0x1FFF) {
            bool newRamEnabled = ((data & 0x0F) == 0x0A);
            if (ramEnabled && !newRamEnabled && isDirty) {
                forceSave = true;
            }
            ramEnabled = newRamEnabled;
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
                    if(hasRam && !eram.empty()) {
                        if (eramBankn[address - 0xA000] != data) {
                            eramBankn[address - 0xA000] = data;
                            isDirty = true;
                        }
                    }
                } else if(bankReg2 >= 0x08 && bankReg2 <= 0x0C) {
                    if (rtcRegs[bankReg2 - 0x08] != data) {
                        rtcRegs[bankReg2 - 0x08] = data;
                        isDirty = true;
                    }
                }
            }
        }
    } else if(mbc == MBC::MBC5) {
        if(address >= 0x0000 && address <= 0x1FFF) {
            bool newRamEnabled = ((data & 0x0F) == 0x0A);
            if (ramEnabled && !newRamEnabled && isDirty) {
                forceSave = true;
            }
            ramEnabled = newRamEnabled;
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
            if(hasRam && ramEnabled && eramBankn) {
                if (eramBankn[address - 0xA000] != data) {
                    eramBankn[address - 0xA000] = data;
                    isDirty = true;
                }
            }
        }
    }
    updateOffsets();
}

bool ICartridge::load_rom(const uint8_t* data, size_t size, bool stream, const char* filename) {
    if(size < 0x8000) return false;

    romSize = size;

    if(!stream) {
        isStreaming = false;
        romData = data;
    } else if(filename != nullptr) {
        isStreaming = true;
        if (!openFileStream(filename)) return false;
        romData = nullptr;
    } else {
        return false;
    }

    mbcMode = 0;
    uint8_t mbcType;
    uint8_t ramSizeCode;

    if(isStreaming) {
        uint8_t header[0x150];
        readStreamHeader(header);
        mbcType = header[0x147];
        ramSizeCode = header[0x149];
        romBank0 = getRomBank(0);
        romBankn = getRomBank(0x4000);
    } else {
        romBank0 = romData;
        romBankn = romData + 0x4000;
        mbcType = romData[0x147];
        ramSizeCode = romData[0x149];
    }

    switch (mbcType) {
    case 0x03:
        hasBattery = true;
        [[fallthrough]];
    case 0x02:
        hasRam = true;
        [[fallthrough]];
    case 0x01:
        mbc = MBC::MBC1;
        break;

    case 0x13:
    case 0x10:
    case 0x0F:
        hasBattery = true;
        [[fallthrough]];
    case 0x12:
        hasRam = true;
        [[fallthrough]];
    case 0x11:
        mbc = MBC::MBC3;
        break;

    case 0x1E:
    case 0x1B:
        hasBattery = true;
        [[fallthrough]];
    case 0x1D:
    case 0x1A:
        hasRam = true;
        [[fallthrough]];
    case 0x1C:
    case 0x19:
        mbc = MBC::MBC5;
        break;
    default:
        mbc = MBC::MBC0;
        break;
    }

    uint32_t ramSize = 0;
    switch(ramSizeCode) {
        case 0x01: ramSize = 8192; break;
        case 0x02: ramSize = 8192; break;
        case 0x03: ramSize = 32768; break;
        case 0x04: ramSize = 131072; break;
        case 0x05: ramSize = 65536; break;
    }
    if (ramSize > 0) {
        eram.resize(ramSize, 0xFF);
    }

    lastOffsetN = 0xFFFFFFFF;
    lastOffset0 = 0xFFFFFFFF;
    bankReg1 = 1;
    bankReg2 = 0;
    updateOffsets();

    if (filename != nullptr && strlen(filename) > 0) {
        std::string baseName = filename;
        size_t lastDot = baseName.find_last_of(".");
        if (lastDot != std::string::npos) {
            currentSaveName = baseName.substr(0, lastDot) + ".sav";
        } else {
            currentSaveName = baseName + ".sav";
        }
    }

    // Checking if save exists is left to load() implementations for safety.
    if (hasBattery && !currentSaveName.empty()) {
        load();
    }

    return true;
}

void ICartridge::updateOffsets() {
    uint32_t romOffsetN = 0x4000;
    uint32_t ramOffset = 0;
    uint32_t romOffset0 = 0;

    switch (mbc) {
    case MBC::MBC0:
        if (!isStreaming) {
            romBank0 = romData;
            romBankn = romData + 0x4000;
        } else {
            if (lastOffset0 != 0) {
                romBank0 = getRomBank(0);
                lastOffset0 = 0;
            }
            if (lastOffsetN != 0x4000) {
                romBankn = getRomBank(0x4000);
                lastOffsetN = 0x4000;
            }
        }
        break;

    case MBC::MBC1:
        {
            uint8_t fullRomBank = bankReg1;
            if (mbcMode == 0) fullRomBank |= (bankReg2 << 5);
            romOffsetN = (fullRomBank % (romSize / 0x4000)) * 0x4000;

            if (!isStreaming) {
                romBankn = romData + romOffsetN;
            } else if (romOffsetN != lastOffsetN){
                romBankn = getRomBank(romOffsetN);
                lastOffsetN = romOffsetN;
            }

            if (mbcMode == 1) {
                romOffset0 = ((bankReg2 << 5) % (romSize / 0x4000)) * 0x4000;
            }

            if (!isStreaming) {
                romBank0 = romData + romOffset0;
            } else if (romOffset0 != lastOffset0){
                romBank0 = getRomBank(romOffset0);
                lastOffset0 = romOffset0;
            }

            if (hasRam && !eram.empty()) {
                uint32_t numBanks = eram.size() / 0x2000;
                uint32_t effectiveRamBank = 0;
                if (mbcMode == 1) effectiveRamBank = bankReg2 % numBanks;
                ramOffset = effectiveRamBank * 0x2000;
                eramBankn = eram.data() + ramOffset;
            }
        }
        break;

    case MBC::MBC3:
        {
            uint16_t fullRomBank = bankReg1;
            romOffsetN = (fullRomBank % (romSize / 0x4000)) * 0x4000;

            if (!isStreaming) {
                romBankn = romData + romOffsetN;
                romBank0 = romData + romOffset0;
            } else  {
                if (romOffset0 != lastOffset0) {
                    romBank0 = getRomBank(romOffset0);
                    lastOffset0 = romOffset0;
                }
                if (romOffsetN != lastOffsetN) {
                    romBankn = getRomBank(romOffsetN);
                    lastOffsetN = romOffsetN;
                }
            }

            if (hasRam && !eram.empty()) {
                if (bankReg2 <= 0x03) {
                    ramOffset = (bankReg2 % (eram.size() / 0x2000)) * 0x2000;
                    eramBankn = eram.data() + ramOffset;
                }
            }
        }
        break;

    case MBC::MBC5:
        {
            uint16_t fullRomBank = (static_cast<uint16_t>(bankReg2 & 0x01) << 8) | bankReg1;
            romOffsetN = (fullRomBank % (romSize / 0x4000)) * 0x4000;

            if (!isStreaming) {
                romBankn = romData + romOffsetN;
                romBank0 = romData + romOffset0;
            } else  {
                if (romOffset0 != lastOffset0) {
                    romBank0 = getRomBank(romOffset0);
                    lastOffset0 = romOffset0;
                }
                if (romOffsetN != lastOffsetN) {
                    romBankn = getRomBank(romOffsetN);
                    lastOffsetN = romOffsetN;
                }
            }

            if (hasRam && !eram.empty()) {
                ramOffset = (mbcMode % (eram.size() / 0x2000)) * 0x2000;
                eramBankn = eram.data() + ramOffset;
            }
        }
        break;

    default:
        break;
    }
}

void ICartridge::addSeconds(uint32_t seconds) {
    if (rtcRegs[4] & 0x40) return;

    uint32_t s = rtcRegs[0] + seconds;
    rtcRegs[0] = s % 60;

    uint32_t m = rtcRegs[1] + (s / 60);
    rtcRegs[1] = m % 60;

    uint32_t h = rtcRegs[2] + (m / 60);
    rtcRegs[2] = h % 24;

    uint16_t d = (rtcRegs[3] | ((rtcRegs[4] & 0x01) << 8)) + (h / 24);
    uint16_t finalDays = d % 512;
    rtcRegs[3] = finalDays & 0xFF;
    rtcRegs[4] = (rtcRegs[4] & 0xFE) | ((finalDays >> 8) & 0x01);

    if (d > 511) {
        rtcRegs[4] |= 0x80;
    }
}

bool ICartridge::getForceSave() {
    bool state = forceSave;
    forceSave = false;
    return state;
}