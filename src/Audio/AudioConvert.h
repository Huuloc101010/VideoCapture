#ifndef AUDIO_CONVERT
#define AUDIO_CONVERT

#include "Define.h"
#include "Log.h"

class AudioConvert
{
public:
    UniqueFramePtr ConvertPacketToFrame(UniquePacketPtr Packet);

private:
};

#endif // AUDIO_DEFINE