#ifndef ALSA_CAPTURE_H
#define ALSA_CAPTURE_H
#include "Define.h"
#include "Log.h"

class AlsaCapture
{
public:
    bool Config(const AlsaCaptureConfig& Config);
    UniquePacketPtr ReadPacket();
    AVStreamPtr GetStream();

private:
    UniqueFormatContext      m_FormatContext = nullptr;
    int                      m_AudioStreamIndex = -1;
    AlsaCaptureConfig        m_Config;
};

#endif // ALSA_CAPTURE_H
