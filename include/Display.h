#pragma once

#include <ILI9341_t3n.h>

class Display 
{
private:
    ILI9341_t3n tft;
    static constexpr uint8_t TFT_CS   = 10;
    static constexpr uint8_t TFT_DC   = 9;
    static constexpr uint8_t TFT_RST  = 8;
    static constexpr uint8_t TFT_MOSI = 11;
    static constexpr uint8_t TFT_SCK  = 13;
    static constexpr uint8_t TFT_MISO = 12;

public:
    Display();
    void init();
    void update(const uint16_t* frameBuffer);
}; 