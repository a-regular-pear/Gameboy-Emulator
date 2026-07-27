#pragma once
#include "IRomLoader.h"
#include <Arduino.h>

class TeensyRomLoader : public IRomLoader
{
private:
    enum class Color : uint16_t {
        white = 0x9DE1, 
        lightGray = 0x8D61, 
        darkGray = 0x3306, 
        black = 0x09C1
    };

    static const int MAX_VISIBLE = 8;
    
    void draw(const String files[], int count, int selected, int offset);
    void bootAnimation(); 

public:
    TeensyRomLoader(IDisplay& d, IJoypad& j);
    std::string selectROM() override;
};