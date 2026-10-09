#ifndef ENCODER_H
#define ENCODER_H

#include <vector>
#include "Define.h"
#include "Log.h"

class Encoder
{
public:
    Encoder() = default;
    ~Encoder() = default;
    virtual bool ConfigEncoder(const EncoderConfig& Config) = 0;
    std::vector<UniquePacketPtr> Encode(UniqueFramePtr Frame);
    const UniqueCodecContext& GetCodecContext() const;
    AVRational GetTimeBase();

protected:
    UniqueCodecContext m_CodecContext = nullptr;
    AVCodecPtr         m_Codec        = nullptr;
};
 
#endif // ENCODER_H
