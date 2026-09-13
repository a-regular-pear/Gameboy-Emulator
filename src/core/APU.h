#pragma once

class APU
{
private:
    uint8_t channel1[5] = {0};       // 0xFF10 - 0xFF14
    uint8_t channel2[4] = {0};       // 0xFF16 - 0xFF19
    uint8_t channel3[5] = {0};       // 0xFF1A - 0xFF1E
    uint8_t channel4[4] = {0};       // 0xFF20 - 0xFF23
    uint8_t global_regs[3] = {0};    // 0xFF24 - 0xFF26
    uint8_t wave_ram[16] = {0};      // 0xFF30 - 0xFF3F
public:

};

