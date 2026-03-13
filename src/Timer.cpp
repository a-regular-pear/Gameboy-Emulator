#include "Timer.h"
#include "Bus.h"

Timer::Timer() :
div(0),
tima(0),
tma(0),
tac(0),
tima_countdown(1024),
bus(nullptr)
{}

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
    else if(address == 0xFF07) {
        tac = value;
        switch (tac & 0x03) {
        case 0: tima_countdown = 1024; break;
        case 1: tima_countdown = 16; break;
        case 2: tima_countdown = 64; break;
        case 3: tima_countdown = 256; break;
        }   
    };
}

void Timer::step(int cycles) {
    //div return only the 8 MSB and is updated after 256 T-cycles (so after div += 0xFF cycles)
    div += cycles;

    //Check if tima is enabled
    if((tac & 0x04) != 0) {
        tima_countdown -= cycles;
        // Check if enough time has passed to update tima
        if(tima_countdown <= 0) {
            //Update countdown
            switch (tac & 0x03) {
            case 0: tima_countdown += 1024; break;
            case 1: tima_countdown += 16; break;
            case 2: tima_countdown += 64; break;
            case 3: tima_countdown += 256; break;
            default: break;
            }

            // Checks for everyflow
            if(tima == 0xFF) {
                tima = tma;
                bus->requestInterrupt(2);
            } else {
                tima++;
            }
        }
    }
}

void Timer::setBus(Bus* b) {
    this->bus = b;
}