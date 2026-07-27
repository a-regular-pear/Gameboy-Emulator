#include "IJoypad.h"

IJoypad::IJoypad() : state{0xFF}, selector{0x30} {}

void IJoypad::setSelector(uint8_t data) {
    selector = data & 0x30;
}

uint8_t IJoypad::getState() const {
    uint8_t res = 0x0F;
    if (!(selector & 0x10)) res &= (state & 0x0F);
    if (!(selector & 0x20)) res &= ((state >> 4) & 0x0F);
    return 0xC0 | selector | res;
}

bool IJoypad::isPressed(Button button) const {
    return !(state & (1 << static_cast<int>(button)));
}