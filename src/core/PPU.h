#pragma once
#include <cstdint>

class Bus;  // forward declaration

class PPU
{
private:
    struct Sprite {
        // This is signed and 16 to be able to have negative height(in oam y = 16 makes the object invisible so it has negative height if we consider y = 0 <=> ly = 0)
        int16_t y;
        // Same reason with y
        int16_t x;
        uint8_t tileIndex;
        uint8_t attributes;

    };

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
    uint8_t spriteInLine;
    uint8_t bgLineColorIds[160];
    Sprite sprites[10];
    uint8_t windowCounter;

    bool lastSignal;
    bool frameReady;
    bool updateMode(uint8_t mode);
    void compareLYC();
    void updateInterrupts();
    Color mapIdToRGB565(uint8_t colorID, uint8_t palette);
    void renderScanline();
    void findSprites();
    void renderSprites();
    void renderWindow();
public:
    PPU();
    
    uint8_t read(uint16_t address);
    void write(uint16_t address, uint8_t data);
    
    void setBus(Bus* bus);
    void step(int cycles);
    void renderBackground();
    uint16_t* getFrameBuffer() { return frameBuffer; }
    bool getFrameReady() const { return frameReady; }
    void setFrameReady(bool ready) { frameReady = ready; }
};

