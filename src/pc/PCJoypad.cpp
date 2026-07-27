#include "PCJoypad.h"
#include <SDL.h>

void PCJoypad::init() {
    // SDL event subsystem is typically initialized in the main display/system module.
    // Keyboard state can be polled directly without separate joystick initialization here.
}

void PCJoypad::checkInput() {
    uint8_t newState = 0xFF;
    
    // Polls the current state of all keys synchronously
    SDL_PumpEvents(); 
    const Uint8* keyboardState = SDL_GetKeyboardState(nullptr);

    // Mapped appropriately for typical PC emulation (Arrows for D-Pad, Z/X for A/B, Return/Space for Start/Select)
    if (keyboardState[SDL_SCANCODE_RIGHT])  newState &= ~(1 << static_cast<int>(Button::Right));
    if (keyboardState[SDL_SCANCODE_LEFT])   newState &= ~(1 << static_cast<int>(Button::Left));
    if (keyboardState[SDL_SCANCODE_UP])     newState &= ~(1 << static_cast<int>(Button::Up));
    if (keyboardState[SDL_SCANCODE_DOWN])   newState &= ~(1 << static_cast<int>(Button::Down));
    
    // B maps to X, A maps to Z
    if (keyboardState[SDL_SCANCODE_X])      newState &= ~(1 << static_cast<int>(Button::B));
    if (keyboardState[SDL_SCANCODE_Z])      newState &= ~(1 << static_cast<int>(Button::A));
    
    if (keyboardState[SDL_SCANCODE_SPACE])  newState &= ~(1 << static_cast<int>(Button::Select));
    if (keyboardState[SDL_SCANCODE_RETURN]) newState &= ~(1 << static_cast<int>(Button::Start));

    state = newState;
}