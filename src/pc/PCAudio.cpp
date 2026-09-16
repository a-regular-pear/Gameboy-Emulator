#include "PCAudio.h"

PCAudio::PCAudio() {
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
        return;
    }

    SDL_AudioSpec want, have;
    SDL_zero(want);

    want.freq = 44100;
    want.format = AUDIO_S16SYS;
    want.channels = 2;
    want.samples = 1024;
    want.callback = PCAudio::audioCallback;
    want.userdata = this;

    device_id = SDL_OpenAudioDevice(nullptr, 0, &want, &have, 0);

    if (device_id == 0) {
        return;
    }

    SDL_PauseAudioDevice(device_id, 0);
}

PCAudio::~PCAudio() {
    if (device_id != 0) {
        SDL_CloseAudioDevice(device_id);
    }
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

void PCAudio::audioCallback(void* userdata, uint8_t* stream, int len) {
    PCAudio* audio = static_cast<PCAudio*>(userdata);
    int16_t* out_stream = reinterpret_cast<int16_t*>(stream);
    int samples_needed = len / sizeof(int16_t);

    int current_read = audio->read_index.load(std::memory_order_relaxed);
    int current_write = audio->write_index.load(std::memory_order_acquire);

    for (int i = 0; i < samples_needed; ++i) {
        if (current_read != current_write) {
            out_stream[i] = audio->ring_buffer[current_read];
            current_read = (current_read + 1) & BUFFER_MASK;
        } else {
            // Buffer underrun: output silence
            out_stream[i] = 0;
        }
    }

    audio->read_index.store(current_read, std::memory_order_release);
}

void PCAudio::pushSample(int16_t left, int16_t right) {
    int current_write = write_index.load(std::memory_order_relaxed);
    int current_read = read_index.load(std::memory_order_acquire);

    int next_write_left = (current_write + 1) & BUFFER_MASK;
    int next_write_right = (next_write_left + 1) & BUFFER_MASK;

    // Check if the ring buffer is full to prevent overflow
    if (next_write_right == current_read) {
        return; // Drop sample on overflow
    }

    ring_buffer[current_write] = left;
    ring_buffer[next_write_left] = right;

    write_index.store(next_write_right, std::memory_order_release);
}

uint32_t PCAudio::getSampleRate() const {
    return 44100; 
}