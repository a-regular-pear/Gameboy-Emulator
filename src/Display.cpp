#include "Display.h"

Display::Display() : tft(TFT_CS, TFT_DC, TFT_RST, TFT_MOSI, TFT_SCK, TFT_MISO) {}

void Display::init() {
    tft.begin();            
    tft.setRotation(1);     
    tft.fillScreen(0x09C1);
}
void Display::update(const uint16_t* frameBuffer) {
    uint16_t lineBuffer[320];
    
    // Constants for the 266x240 centered image
    const int targetW = 266;
    const int targetH = 240;
    const int offsetX = (320 - targetW) / 2; // 27 pixels

    // Fixed-point scale factors (16.16)
    uint32_t x_step = (160 << 16) / targetW;
    uint32_t y_step = (144 << 16) / targetH;

    for (int screenY = 0; screenY < targetH; screenY++) {
        uint32_t sourceY = (screenY * y_step) >> 16;
        const uint16_t* sourceRow = &frameBuffer[sourceY * 160];
        
        uint32_t x_acc = 0;
        for (int screenX = 0; screenX < targetW; screenX++) {
            lineBuffer[screenX] = sourceRow[x_acc >> 16];
            x_acc += x_step;
        }

        // Write the scaled line starting at the offset
        tft.writeRect(offsetX, screenY, targetW, 1, lineBuffer);
    }
}