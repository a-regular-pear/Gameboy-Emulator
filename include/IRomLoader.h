#pragma once
#include <string>

class IDisplay;
class IJoypad;

class IRomLoader
{
protected:
    IDisplay& display;
    IJoypad& joypad;

public:
    IRomLoader(IDisplay& d, IJoypad& j) : display(d), joypad(j) {}
    virtual ~IRomLoader() = default;
    
    virtual std::string selectROM() = 0;
};