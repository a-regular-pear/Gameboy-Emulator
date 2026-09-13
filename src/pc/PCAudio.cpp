#include "PCAudio.h"

PCAudio::PCAudio() {
    // Initialize the SDL audio subsystem
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
        // Handle initialization error
        return;
    }

    // Set up the desired audio hardware specifications
    SDL_AudioSpec want, have;
    SDL_zero(want);

    want.freq = 44100;              // Sampling rate in Hz
    want.format = AUDIO_S16SYS;     // 16-bit signed integer (system byte order)
    want.channels = 2;              // Stereo audio (Left and Right)
    want.samples = 1024;            // Audio buffer size in frames
    want.callback = PCAudio::audioCallback; // Static callback function
    want.userdata = this;           // Pass the current class instance to the callback

    // Open the default audio playback device
    // nullptr: default device, 0: no changes to specifications allowed
    device_id = SDL_OpenAudioDevice(nullptr, 0, &want, &have, 0);

    if (device_id == 0) {
        // Handle failure to open the audio device
        return;
    }

    // The audio device starts paused; unpause it to begin calling the callback
    SDL_PauseAudioDevice(device_id, 0);
}

PCAudio::~PCAudio() {
    // Safely close the audio device and quit the subsystem upon destruction
    if (device_id != 0) {
        SDL_CloseAudioDevice(device_id);
    }
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

void PCAudio::audioCallback(void* userdata, uint8_t* stream, int len) {
    // Cast the user data back to the current PCAudio instance
    PCAudio* audio = static_cast<PCAudio*>(userdata);

    // Treat the output buffer as a 16-bit audio sample buffer
    int16_t* out_stream = reinterpret_cast<int16_t*>(stream);

    // 'len' is given in bytes, so convert it to the number of int16_t samples
    int samples_needed = len / sizeof(int16_t);

    // Mask used for efficient wrap-around in the circular buffer
    const int MASK = BUFFER_CAPACITY - 1;

    for (int i = 0; i < samples_needed; ++i) {
        // Check if there is data available in the ring buffer
        if (audio->read_index != audio->write_index) {
            // Copy the next sample to the SDL output buffer
            out_stream[i] = audio->ring_buffer[audio->read_index];

            // Advance the read index with circular wrap-around
            audio->read_index = (audio->read_index + 1) & MASK;
        } else {
            // Buffer underrun: the emulator is not producing audio fast enough.
            // Output silence to avoid playing garbage data.
            out_stream[i] = 0;
        }
    }
}

void PCAudio::pushSample(int16_t left, int16_t right) {
    const int MASK = BUFFER_CAPACITY - 1;

    ring_buffer[write_index] = left;
    write_index = (write_index + 1) & MASK;

    ring_buffer[write_index] = right;
    write_index = (write_index + 1) & MASK;
}