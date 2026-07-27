#include "TeensyDisplay.h"

TeensyDisplay::TeensyDisplay() : tft(TFT_CS, TFT_DC, TFT_RST, TFT_MOSI, TFT_SCK, TFT_MISO) {}

void TeensyDisplay::init() {
    tft.begin(40000000);      
    tft.useFrameBuffer(true); 
    tft.setRotation(1);     
    tft.fillScreen(0x09C1);
}
void TeensyDisplay::update(const uint16_t* frameBuffer) {
    // Access the internal frame buffer pointer
   uint16_t* screen = tft.getFrameBuffer();


    if (screen == nullptr) return; // Safety check

    const int targetW = 266;
    const int targetH = 240;
    const int offsetX = (320 - targetW) / 2;

    uint32_t x_step = (160 << 16) / targetW;
    uint32_t y_step = (144 << 16) / targetH;
 

    // Manual scaling into the RAM buffer
    for (int screenY = 0; screenY < targetH; screenY++) {
        uint32_t sourceY = (screenY * y_step) >> 16;
        const uint16_t* sourceRow = &frameBuffer[sourceY * 160];
        
        // Calculate destination in the 320x240 screen buffer
        uint16_t* destRow = &screen[screenY * 320 + offsetX];
        
        uint32_t x_acc = 0;
        for (int screenX = 0; screenX < targetW; screenX++) {
            destRow[screenX] = sourceRow[x_acc >> 16];
            x_acc += x_step;
        }
    }

    // Update the actual hardware
    tft.updateScreenAsync(); 
}