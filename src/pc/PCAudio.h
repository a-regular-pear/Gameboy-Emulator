#pragma once
#include "IAudio.h"
#include <vector>
#include <SDL.h>
#include <atomic>

class PCAudio : public IAudio
{
private:
    // 1024 samples * 2 channels = 2048 integers
    static constexpr int BUFFER_CAPACITY = 2048; 
    
    int16_t ring_buffer[BUFFER_CAPACITY];
    std::atomic<int> write_index{0};
    std::atomic<int> read_index{0};

    SDL_AudioDeviceID device_id = 0;
    static void audioCallback(void* userdata, uint8_t* stream, int len);
public:
    PCAudio(); // Constructor handles SDL initialization
    ~PCAudio() override; // Destructor handles SDL cleanup

    void pushSample(int16_t left, int16_t right) override;
};