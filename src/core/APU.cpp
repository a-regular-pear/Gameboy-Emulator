#include "APU.h"
#include <cmath>
#include <algorithm>

void APU::set_audio_backend(IAudio* backend) {
    audio_backend = backend;
    
    if (audio_backend != nullptr) {
        uint32_t rate = audio_backend->getSampleRate();
        
        // Scale the RC filter decay factor from the 4.19 MHz base clock to the target audio sample rate
        charge_factor = std::pow(0.999958f, 4194304.0f / rate);
    }
}

void APU::mix_audio(float& out_left, float& out_right) {
    float left_mix = 0.0f;
    float right_mix = 0.0f;
    
    float ch1_out = channel1.get_analog_output();
    float ch2_out = channel2.get_analog_output();
    
    // NR51 panning mask for Channel 1 (Bit 4 Left, Bit 0 Right)
    if ((nr51 & 0x10) != 0) left_mix += ch1_out;
    if ((nr51 & 0x01) != 0) right_mix += ch1_out;

    // NR51 panning mask for Channel 2 (Bit 5 Left, Bit 1 Right)
    if ((nr51 & 0x20) != 0) left_mix += ch2_out;
    if ((nr51 & 0x02) != 0) right_mix += ch2_out;
    
    // Calculate master volume multiplier (NR50)
    float vol_left = (((nr50 >> 4) & 0x07) + 1) / 8.0f;
    float vol_right = ((nr50 & 0x07) + 1) / 8.0f;
    
    left_mix *= vol_left;
    right_mix *= vol_right;
    
    // Apply independent AC coupling per terminal
    out_left = left_mix - capacitor_left;
    capacitor_left = left_mix - out_left * charge_factor;
    
    out_right = right_mix - capacitor_right;
    capacitor_right = right_mix - out_right * charge_factor;
}
void APU::step(int t_cycles, uint8_t current_div) {
    if (!apu_enabled) {
        return;
    }

    static int t_accumulator = 0;
    t_accumulator += t_cycles;

    while (t_accumulator >= 4) {
        t_accumulator -= 4;

        channel1.tick();
        channel2.tick();

        bool current_div_bit = (current_div & 0x10) != 0;

        if (previous_div_bit && !current_div_bit) {
            if (div_apu_tick % 2 == 0) {
                channel1.clock_length();
                channel2.clock_length();
            }

            if (div_apu_tick % 4 == 2) {
                channel1.clock_sweep();
            }

            if (div_apu_tick % 8 == 7) {
                channel1.clock_envelope();
                channel2.clock_envelope();
            }

            div_apu_tick = (div_apu_tick + 1) % 8;
        }

        previous_div_bit = current_div_bit;

        if (audio_backend != nullptr) {
            sample_counter += audio_backend->getSampleRate();
            
            if (sample_counter >= 1048576) {
                sample_counter -= 1048576;
                
                float analog_left = 0.0f;
                float analog_right = 0.0f;
                
                mix_audio(analog_left, analog_right);
                
                analog_left *= 0.25f;
                analog_right *= 0.25f;
                
                analog_left = std::clamp(analog_left, -1.0f, 1.0f);
                analog_right = std::clamp(analog_right, -1.0f, 1.0f);

                int16_t pcm_left = static_cast<int16_t>(analog_left * 32767.0f);
                int16_t pcm_right = static_cast<int16_t>(analog_right * 32767.0f);
                
                audio_backend->pushSample(pcm_left, pcm_right);
            }
        }
    }
}
uint8_t APU::read_register(uint16_t address) const {
    if (address >= 0xFF10 && address <= 0xFF14) {
        return channel1.read(address);
    }

    if (address >= 0xFF16 && address <= 0xFF19) {
        return channel2.read(address);
    }
    
    if (address == 0xFF24) return nr50;
    if (address == 0xFF25) return nr51;

    // Master audio control (NR52)
    if (address == 0xFF26) {
            // Unused bits 4-6 always read as high
            uint8_t status = 0x70; 
            
            if (apu_enabled) {
                status |= 0x80;
            }
            
        // Channel active flags (Bit 0 for CH1, Bit 1 for CH2)
        if (channel1.is_active()) status |= 0x01; 
        if (channel2.is_active()) status |= 0x02;  
            
            return status;
    }
    // Unmapped or unimplemented audio registers return 0xFF
    return 0xFF;
}

void APU::write_register(uint16_t address, uint8_t value) {
    // Master audio control (NR52)
    if (address == 0xFF26) {
        bool power_on = (value & 0x80) != 0;
        
        if (!power_on && apu_enabled) {
            apu_enabled = false;
            
            // Hard power-off clears all APU registers
            nr50 = 0x00;
            nr51 = 0x00;
            // Channel reset functions should be called here
        } else if (power_on && !apu_enabled) {
            apu_enabled = true;
        }
        
        return;
    }
    
    // Ignore writes to audio registers if the APU is powered down
    if (!apu_enabled) {
        return;
    }

    if (address == 0xFF24) {
        nr50 = value;
        return;
    }

    if (address == 0xFF25) {
        nr51 = value;
        return;
    }

    if (address >= 0xFF10 && address <= 0xFF14) {
        channel1.write(address, value);
    }

    if (address >= 0xFF16 && address <= 0xFF19) {
        channel2.write(address, value);
    }

}

