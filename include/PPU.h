#pragma once
#include <cstdint>

class Bus;  // forward declaration

class PPU
{
private:

    enum class Color : uint16_t {white = 0x9DE1, lightGray = 0x8D61, darkGray = 0x3306, black = 0x09C1};
    uint16_t frameBuffer[160 * 144]; // RGB565 format

    //This is a pointer and not a reference to avoid issues of circular dependency
    Bus* bus;

    uint8_t vram[0x2000];
    uint8_t oam[0xA0];

    uint16_t dotLineCounter;
    uint8_t lcdc;
    uint8_t stat;
    uint8_t scy, scx;
    uint8_t ly;
    uint8_t lyc;
    uint8_t bgp, obp0, obp1;
    uint8_t wy, wx;

    bool lastSignal;
    bool frameReady;
    bool updateMode(uint8_t mode);
    void compareLYC();
    void updateInterrupts();
    Color mapIdToRGB565(uint8_t colorID);

public:
    PPU();
    
    uint8_t read(uint16_t address);
    void write(uint16_t address, uint8_t data);
    
    void setBus(Bus* bus);
    void step(int cycles);
    void renderScanline();
    uint16_t* getFrameBuffer() { return frameBuffer; }
    bool getFrameReady() const { return frameReady; }
    void setFrameReady(bool ready) { frameReady = ready; }
};

