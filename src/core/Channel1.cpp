#include "Channel1.h"

uint8_t Channel1::read(uint16_t address) const {
    switch (address) {
        case 0xFF10:
            return (pitch_sweep_pace << 4) | (pitch_sweep_direction << 3) | pitch_sweep_step | 0x80;
        case 0xFF11:
            return (duty_cycle << 6) | 0x3F;
        case 0xFF12:
            return (initial_volume << 4) | (envelope_direction << 3) | sweep_pace;
        case 0xFF13:
            return 0xFF; // Write-only register
        case 0xFF14:
            return (length_enable ? 0x40 : 0x00) | 0xBF;
        default:
            return 0xFF;
    }
}

void Channel1::write(uint16_t address, uint8_t value) {
    switch (address) {
        case 0xFF10:
            pitch_sweep_pace = (value >> 4) & 0x07;
            pitch_sweep_direction = (value >> 3) & 0x01;
            pitch_sweep_step = value & 0x07;
            break;
        case 0xFF11:
            duty_cycle = (value >> 6) & 0x03;
            length_timer = value & 0x3F; 
            break;
        case 0xFF12:
            initial_volume = (value & 0xF0) >> 4;
            envelope_direction = (value & 0x08) >> 3;
            sweep_pace = value & 0x07;
            dac_enabled = (value & 0xF8) != 0;
            break;
        case 0xFF13:
            period_value = (period_value & 0x0700) | value;
            break;
        case 0xFF14:
            period_value = (period_value & 0x00FF) | ((value & 0x07) << 8);
            length_enable = (value & 0x40) != 0;
            if ((value & 0x80) != 0) {
            
                channel_enabled = true;
                
                if (length_timer == 0) {
                    length_timer = 64;
                }
                
                internal_period_timer = 2048 - period_value;
                current_volume = initial_volume;
                
                envelope_timer = sweep_pace;
                if (envelope_timer == 0) {
                    envelope_timer = 8;
                }

                // Initialize sweep unit
                shadow_period = period_value;
                sweep_timer = (pitch_sweep_pace > 0) ? pitch_sweep_pace : 8;
                sweep_enabled = (pitch_sweep_pace > 0 || pitch_sweep_step > 0);
                
                if (pitch_sweep_step > 0) {
                    calculate_sweep_period();
                }

                if (!dac_enabled) {
                    channel_enabled = false;
                }
            }
            break;    
        default:
            break;
    }
}

void Channel1::calculate_sweep_period() {
    uint16_t next_period_offset = shadow_period >> pitch_sweep_step;
    uint16_t next_period = shadow_period;
    
    if(pitch_sweep_direction == 1) {
        next_period -= next_period_offset;
    } else {
        next_period += next_period_offset;
    }

    // Disable channel if the period overflows 2047
    if (next_period > 2047) {
        channel_enabled = false;
    } else if (pitch_sweep_step > 0) {
        shadow_period = next_period;
        period_value = next_period; 

        // Extra hardware overflow check for addition operations
        uint16_t next_offset = shadow_period >> pitch_sweep_step;
        uint16_t next_check = shadow_period;
        
        if (pitch_sweep_direction == 0) { 
            next_check += next_offset;
            if (next_check > 2047) {
                channel_enabled = false;
            }
        }
    }
}

void Channel1::clock_sweep() {
    if (sweep_timer > 0) {
        sweep_timer--;
    }

    if (sweep_timer == 0) {
        sweep_timer = (pitch_sweep_pace > 0) ? pitch_sweep_pace : 8;

        if (sweep_enabled && pitch_sweep_pace > 0) {
            calculate_sweep_period();
        }
    }
}

void Channel1::tick() {
    internal_period_timer--;
    if(internal_period_timer <= 0) {
            // Reload the timer
            internal_period_timer = 2048 - period_value;

            // Advance the 8-step sequencer
            sequence_pointer = (sequence_pointer + 1) % 8;

            if (channel_enabled && dac_enabled) {
                current_output = duty_waveforms[duty_cycle][sequence_pointer];
            } else {
                current_output = 0;
            }
    }
}

void Channel1::clock_length() {
    // Only decrement if the length timer is enabled and greater than zero
    if (length_enable && length_timer > 0) {
        length_timer--;
        
        if (length_timer == 0) {
            // Timer expired, disable the channel playback
            channel_enabled = false;
        }
    }
}

void Channel1::clock_envelope() {
    // If sweep pace is 0, the volume remains constant
    if (sweep_pace != 0) {
        envelope_timer--;
        
        if (envelope_timer == 0) {
            // Reload the timer with the pace value
            envelope_timer = sweep_pace;
            
            // Adjust volume based on direction, bounded between 0 and 15
            if (envelope_direction == 1 && current_volume < 15) {
                current_volume++;
            } else if (envelope_direction == 0 && current_volume > 0) {
                current_volume--;
            }
        }
    }
}

float Channel1::get_analog_output() const {
    if (!dac_enabled) {
        return 0.0f;
    }

    uint8_t digital_value = 0;
    
    if (channel_enabled) {
        digital_value = current_output * current_volume;
    }
    
    return (-2.0f / 15.0f) * digital_value + 1.0f;
}

bool Channel1::is_active() const {
    return channel_enabled;
}