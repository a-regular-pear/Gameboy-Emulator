#pragma once
#include <cstdint>
#include <Arduino.h>

class Joypad
{

private:
    enum class Button : uint8_t {Right = 0, Left = 1, Up = 2, Down = 3, B = 4, A = 5, Select = 6, Start = 7};
    uint8_t state;
    uint8_t selector;

public:
    Joypad();
    void init();
    void checkInput();
    void setSelector(uint8_t data);
    uint8_t getState() const;

};

