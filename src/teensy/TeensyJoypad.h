#pragma once
#include "IJoypad.h"
#include <Arduino.h>

class TeensyJoypad : public IJoypad
{
public:
    void init() override;
    void checkInput() override;
};