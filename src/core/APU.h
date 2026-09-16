#pragma once
#include "IAudio.h"
#include "Channel1.h"
#include "Channel2.h"
#include <cstdint>

class APU {
private:
    IAudio* audio_backend = nullptr;    

    Channel1 channel1;
    Channel2 channel2;

    uint8_t div_apu_tick = 0;
    // Tracks the state of DIV bit 4 in the previous cycle
    bool previous_div_bit = false;

    float capacitor_left = 0.0f;
    float capacitor_right = 0.0f;
    float charge_factor = 0.999958f;

    uint32_t sample_counter = 0;

    void mix_audio(float& out_left, float& out_right);
    
    uint8_t nr50 = 0x00;
    uint8_t nr51 = 0x00;
    bool apu_enabled = false;
    public:
public:
    void set_audio_backend(IAudio* backend);

    void write_register(uint16_t address, uint8_t value);
    uint8_t read_register(uint16_t address) const;
    void step(int t_cycles,uint8_t current_div);
};