#pragma once
#include <stdint.h>

class IDisplay 
{
public:
    virtual ~IDisplay() = default;
    
    virtual void init() = 0;
    virtual void update(const uint16_t* frameBuffer) = 0;
};