#include "Joypad.h"

Joypad::Joypad() : state{0xFF}, selector{0x30} {}

void Joypad::init() {
    for (int i = 0; i < 8; i++) {
        pinMode(i, INPUT);
    }

    
}

void Joypad::checkInput() {
        uint8_t newState = 0xFF;
        if (digitalRead(static_cast<int>(Button::Right)) == HIGH) newState &= ~(1 << static_cast<int>(Button::Right)); 
        if (digitalRead(static_cast<int>(Button::Left)) == HIGH) newState &= ~(1 << static_cast<int>(Button::Left)); 
        if (digitalRead(static_cast<int>(Button::Down)) == HIGH) newState &= ~(1 << static_cast<int>(Button::Down)); 
        if (digitalRead(static_cast<int>(Button::Up)) == HIGH) newState &= ~(1 << static_cast<int>(Button::Up)); 
        if (digitalRead(static_cast<int>(Button::B)) == HIGH) newState &= ~(1 << static_cast<int>(Button::B)); 
        if (digitalRead(static_cast<int>(Button::A)) == HIGH) newState &= ~(1 << static_cast<int>(Button::A)); 
        if (digitalRead(static_cast<int>(Button::Select)) == HIGH) newState &= ~(1 << static_cast<int>(Button::Select)); 
        if (digitalRead(static_cast<int>(Button::Start)) == HIGH) newState &= ~(1 << static_cast<int>(Button::Start)); 
        state = newState;
}

void Joypad::setSelector(uint8_t data) {
        selector = data & 0x30;
}

uint8_t Joypad::getState() const {
        uint8_t res = 0x0F;
        if (!(selector & 0x10)) res &= (state & 0x0F);
        if (!(selector & 0x20)) res &= ((state >> 4) & 0x0F);
        return 0xC0 | selector | res;
}