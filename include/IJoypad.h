#pragma once
#include <cstdint>

class IJoypad
{
protected:
    uint8_t state;
    uint8_t selector;

public:
    enum class Button : uint8_t {Right = 0, Left = 1, Up = 2, Down = 3, B = 4, A = 5, Select = 6, Start = 7};

    IJoypad();
    virtual ~IJoypad() = default;

    virtual void init() = 0;
    virtual void checkInput() = 0;

    void setSelector(uint8_t data);
    uint8_t getState() const;
    bool isPressed(Button button) const;
};