#pragma once
#include <cstdint>

class Timer
{
private:
    uint16_t div;
    uint8_t tima;
    uint8_t tma;
    uint8_t tac;

public:
    Timer(/* args */);
    uint8_t read(uint16_t address);
    void Timer::write(uint16_t address, uint8_t value);
};

