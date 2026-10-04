#include "TeensyAudio.h"

uint32_t TeensyAudio::getSampleRate() const {
    return 44100;
}

void TeensyAudio::pushSample(int16_t left, int16_t right) {
    // Hardware-specific I2S or DAC audio buffering implementation
}