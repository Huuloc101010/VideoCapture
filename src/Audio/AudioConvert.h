#ifndef AUDIO_CONVERT
#define AUDIO_CONVERT

#include "Define.h"
#include "Log.h"

class AudioConvert
{
public:
    bool Config(const AudioConvertConfig& Config);
    UniqueFramePtr ConvertPacketToFrame(UniquePacketPtr Packet);
    // Convert from S16P to Floating point packed
    UniqueFramePtr ConvertS16ToFPTP(UniqueFramePtr Frame);

private:
    AudioConvertConfig m_Config;
    UniqueSwrContext   m_SwrContext;
};

#endif // AUDIO_DEFINE