#include "Display.h"

Display::Display() : tft(TFT_CS, TFT_DC, TFT_RST, TFT_MOSI, TFT_SCK, TFT_MISO) {}

void Display::init() {
    tft.begin();            
    tft.setRotation(1);     
    tft.fillScreen(ILI9341_BLACK);
}
void Display::update(const uint16_t* frameBuffer) {
    uint16_t lineBuffer[320];


    for (int y = 0; y < 120; y++) {
        for (int x = 0; x < 160; x++) {
            uint16_t pixel = frameBuffer[y * 160 + x];
            lineBuffer[x * 2] = pixel;
            lineBuffer[x * 2 + 1] = pixel;
        }

        tft.writeRect(0, y * 2, 320, 1, lineBuffer);
        tft.writeRect(0, y * 2 + 1, 320, 1, lineBuffer);
    }
}