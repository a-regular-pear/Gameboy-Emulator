#pragma once
#include "IJoypad.h"

class PCJoypad : public IJoypad
{
public:
    void init() override;
    void checkInput() override;
};