#include "IAudio.h"

class TeensyAudio : public IAudio {
public:
    TeensyAudio() = default;
    ~TeensyAudio() = default;

    uint32_t getSampleRate() const override;
    void pushSample(int16_t left, int16_t right) override;
};