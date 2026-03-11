#include <Timer.h>

Timer::Timer() {};

uint8_t Timer::read(uint16_t address) {
    if(address == 0xFF04) return static_cast<uint8_t>(div >> 8);
    else if(address == 0xFF05) return tima;
    else if(address == 0xFF06) return tma;
    else if(address == 0xFF07) return tac;

    return 0xFF;
}

void Timer::write(uint16_t address, uint8_t value) {
    if(address == 0xFF04) {
        div = 0;
    }
    else if(address == 0xFF05) tima = value;
    else if(address == 0xFF06) tma = value;
    else if(address == 0xFF07) tac = value;
}