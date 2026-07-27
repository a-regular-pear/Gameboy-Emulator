#pragma once
#include "IDisplay.h"
#include <ILI9341_t3n.h>

class TeensyDisplay : public IDisplay
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
    TeensyDisplay();
    void init() override;
    void update(const uint16_t* frameBuffer) override;
    ILI9341_t3n& getTFT() { return tft; }
};