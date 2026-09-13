#pragma once
#include <cstdint>

class IAudio 
{
public:
    virtual ~IAudio() = default;
    virtual void pushSample(int16_t left, int16_t right) = 0;
};