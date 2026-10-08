#ifndef VIDEO_ENCODER
#define VIDEO_ENCODER

#include "Encoder.h"

class VideoEncoder : public Encoder
{
public:
    VideoEncoder();
    ~ VideoEncoder();
    virtual bool ConfigEncoder(const EncoderConfig& Config) override;
private:

};

#endif // VIDEO_ENCODER