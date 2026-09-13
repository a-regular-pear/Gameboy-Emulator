#pragma once
#include <cstdint>

class Channel2
{
private:

    // Duty cycle waveforms: 12.5%, 25%, 50%, 75%
    const uint8_t duty_waveforms[4][8] = {
        {0, 0, 0, 0, 0, 0, 0, 1}, 
        {1, 0, 0, 0, 0, 0, 0, 1}, 
        {1, 0, 0, 0, 0, 1, 1, 1}, 
        {0, 1, 1, 1, 1, 1, 1, 0}  
    };

    uint8_t duty_cycle = 0;
    uint8_t length_timer = 0;
    uint8_t initial_volume = 0;
    uint8_t envelope_direction = 0;
    uint8_t sweep_pace = 0;
    uint16_t period_value = 0;
    bool length_enable = false;
    bool dac_enabled = false;

    int internal_period_timer = 0;
    uint8_t sequence_pointer = 0;
    uint8_t current_output = 0; 
    uint8_t current_volume = 0;
public:
    uint8_t read(uint16_t address) const;
    void write(uint16_t address, uint8_t value);
    void tick();
};


