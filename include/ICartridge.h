#pragma once
#include <vector>
#include <cstdint>
#include <string>

class ICartridge
{
protected:
    enum class MBC : uint8_t { MBC0 = 0, MBC1 = 1, MBC2 = 2, MBC3 = 3, MBC4 = 4, MBC5 = 5 };

    bool isStreaming;
    const uint8_t* romData;
    size_t romSize;
    const uint8_t* romBank0;
    const uint8_t* romBankn;

    std::vector<uint8_t> eram;
    uint8_t* eramBankn;
    bool hasRam;
    bool ramEnabled;
    bool hasBattery;
    uint8_t mbcMode;
    uint8_t bankReg1;
    uint8_t bankReg2;
    MBC mbc;
    uint32_t lastOffset0;
    uint32_t lastOffsetN;

    uint8_t rtcRegs[5];
    uint8_t rtcLatchedRegs[5];
    uint8_t rtcLatchedValue;

    std::string currentSaveName;
    bool isDirty;
    bool forceSave;

    // Platform-specific abstractions
    virtual const uint8_t* getRomBank(uint32_t offset) = 0;
    virtual bool openFileStream(const char* filename) = 0;
    virtual void readStreamHeader(uint8_t* header) = 0;
    virtual uint32_t getPlatformTime() = 0;

    void updateOffsets();

public:
    ICartridge();
    virtual ~ICartridge() = default;

    bool load_rom(const uint8_t* data, size_t size, bool stream = false, const char* filename = "");
    uint8_t read(uint16_t address);
    void write(uint16_t address, uint8_t data);
    void addSeconds(uint32_t seconds);

    uint8_t getMBC() const { return static_cast<uint8_t>(mbc); }
    bool getIsDirty() const { return isDirty; }
    void setIsDirty(bool dirty) { isDirty = dirty; }
    bool gethasBattery() const { return hasBattery; }
    bool getForceSave();

    virtual void save() = 0;
    virtual void load() = 0;
};