#include "TeensyJoypad.h"

void TeensyJoypad::init() {
    for (int i = 0; i < 7; i++) {
        pinMode(i, INPUT_PULLUP);
    }
    pinMode(33, INPUT_PULLUP);
}

void TeensyJoypad::checkInput() {
    uint8_t newState = 0xFF;
    if (digitalRead(static_cast<int>(Button::Right)) == LOW) newState &= ~(1 << static_cast<int>(Button::Right)); 
    if (digitalRead(static_cast<int>(Button::Left)) == LOW) newState &= ~(1 << static_cast<int>(Button::Left)); 
    if (digitalRead(static_cast<int>(Button::Down)) == LOW) newState &= ~(1 << static_cast<int>(Button::Down)); 
    if (digitalRead(static_cast<int>(Button::Up)) == LOW) newState &= ~(1 << static_cast<int>(Button::Up)); 
    if (digitalRead(static_cast<int>(Button::B)) == LOW) newState &= ~(1 << static_cast<int>(Button::B)); 
    if (digitalRead(static_cast<int>(Button::A)) == LOW) newState &= ~(1 << static_cast<int>(Button::A)); 
    if (digitalRead(static_cast<int>(Button::Select)) == LOW) newState &= ~(1 << static_cast<int>(Button::Select)); 
    if (digitalRead(33) == LOW) newState &= ~(1 << static_cast<int>(Button::Start)); 
    state = newState;
}