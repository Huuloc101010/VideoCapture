#ifndef AUDIO_ENCODER
#define AUDIO_ENCODER

#include "Encoder.h"

class AudioEncoder : public Encoder
{
public:
    AudioEncoder() = default;
    ~AudioEncoder() = default;
    bool ConfigEncoder(const EncoderConfig& Config) override;

private:
};

#endif // AUDIO_ENCODER