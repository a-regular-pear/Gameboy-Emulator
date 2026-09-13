#include "Channel2.h"

uint8_t Channel2::read(uint16_t address) const {
    switch (address) {
        case 0xFF16:
            return (duty_cycle << 6) | 0x3F;
        case 0xFF17:
            return (initial_volume << 4) | (envelope_direction << 3) | sweep_pace;
        case 0xFF18:
            return 0xFF; // Write-only register
        case 0xFF19:
            return (length_enable ? 0x40 : 0x00) | 0xBF;
        default:
            return 0xFF;
    }
}

void Channel2::write(uint16_t address, uint8_t value) {
    switch (address) {
        case 0xFF16:
            duty_cycle = (value >> 6) & 0x03;
            length_timer = value & 0x3F; 
            break;
        case 0xFF17:
            initial_volume = (value & 0xF0) >> 4;
            envelope_direction = (value & 0x08) >> 3;
            sweep_pace = value & 0x07;
            dac_enabled = (value & 0xF8) != 0;
            break;
        case 0xFF18:
            period_value = (period_value & 0x0700) | value;
            break;
        case 0xFF19:
            period_value = (period_value & 0x00FF) | ((value & 0x07) << 8);
            length_enable = (value & 0x40) != 0;
            if ((value & 0x80) != 0) {
                // Trigger event: restart channel, reload timers
            }
            break;    
        default:
            break;
    }
}

void Channel2::tick() {
    internal_period_timer--;
    if(internal_period_timer <= 0) {
            // Reload the timer
            internal_period_timer = 2048 - period_value;
            
            // Advance the 8-step sequencer
            sequence_pointer = (sequence_pointer + 1) % 8;

            current_output = duty_waveforms[duty_cycle][sequence_pointer];
    }
}