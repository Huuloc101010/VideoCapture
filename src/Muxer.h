#ifndef MUXER_H
#define MUXER_H
#include "Define.h"
#include "Log.h"

class Muxer
{
public:
    bool Config(const MuxerConfig& Config);
    bool WritePacket(const UniquePacketPtr Packet);
    bool WriteHeader();
    bool WriteTrailer();
private:
    UniqueFormatContext m_FormatContext;
    AVStreamPtr         m_VideoStream;
    MuxerConfig         m_Config;
};

#endif // MUXER_H