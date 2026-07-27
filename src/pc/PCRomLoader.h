#pragma once
#include "IRomLoader.h"
#include <vector>
#include <string>
#include <cstdint>

class PCRomLoader : public IRomLoader
{
    static const int MAX_VISIBLE = 8;
    std::vector<uint16_t> frameBuffer;

public:
    PCRomLoader(IDisplay& d, IJoypad& j);
    std::string selectROM() override;

private:
    void bootAnimation();
    void draw(const std::vector<std::string>& files, int selected, int offset);

    void clear(uint16_t color);
    void fillRect(int x, int y, int w, int h, uint16_t color);
    void drawChar(int x, int y, char c, uint16_t color);
    void drawText(int x, int y, const std::string& text, uint16_t color);
};