#pragma once
#include <cstdint>
#include <Bus.h>

class Bus;  // forward declaration

class Timer
{
private:
    //This is a pointer and not a reference to avoid issues of circular dependency
    Bus* bus;

    uint16_t div;
    uint8_t tima;
    //Timer for how many cycles are left until tima is updated
    int tima_countdown;
    uint8_t tma;
    uint8_t tac;

public:
    Timer();
    void setBus(Bus* bus);
    uint8_t read(uint16_t address);
    void write(uint16_t address, uint8_t value);
    void step(int cycles);
};

