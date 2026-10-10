#ifndef MUXER_H
#define MUXER_H

#include <mutex>
#include "Define.h"
#include "Log.h"
#include "Clock.h"

class Muxer
{
public:
    bool Config(const MuxerConfig& Config);
    bool ConfigVideo();
    bool ConfigAudio();
    bool WriteVideoPacket(const UniquePacketPtr Packet);
    bool WriteAudioPacket(const UniquePacketPtr Packet);
    bool WriteHeader();
    bool WriteTrailer();
    AVRational GetVideoTimeBase();
    AVRational GetAudioTimeBase();
private:
    UniqueFormatContext m_FormatContext;
    AVStreamPtr         m_VideoStream;
    AVStreamPtr         m_AudioStream;
    MuxerConfig         m_Config;
    std::mutex          m_MutexMuxer;
};

#endif // MUXER_H