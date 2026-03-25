#pragma once

#include <Arduino.h>
#include <SD.h>
#include "Display.h"
#include "Joypad.h"

class RomLoader {
private:
    enum class Color : uint16_t {
        white = 0x9DE1, 
        lightGray = 0x8D61, 
        darkGray = 0x3306, 
        black = 0x09C1
    };

    Display& display;
    Joypad& joypad;
    
    static const int MAX_VISIBLE = 8;
    void draw(const String files[], int count, int selected, int offset);
    void bootAnimation(); 

public:
    RomLoader(Display& d, Joypad& j);
    String selectROM();
};