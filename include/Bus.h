#pragma once
#include <cstdint>

class Bus {
private:
    // TODO Implement RAM 
public:
    uint8_t read(uint16_t address);
    void write(uint16_t address, uint8_t value);
};