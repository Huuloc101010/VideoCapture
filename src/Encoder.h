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
    virtual bool ConfigEncoder(EncoderConfig Config);
    std::vector<UniquePacketPtr> Encode(UniqueFramePtr Frame);

protected:
    UniqueCodecContext m_CodecContext = nullptr;
    AVCodecPtr         m_Codec        = nullptr;
};
 
#endif // ENCODER_H
