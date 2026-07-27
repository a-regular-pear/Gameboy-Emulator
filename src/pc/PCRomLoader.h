#pragma once
#include "IRomLoader.h"

class PCRomLoader : public IRomLoader
{
public:
    PCRomLoader(IDisplay& d, IJoypad& j);
    std::string selectROM() override;
};