#ifndef V4L2_CAPTURE_H
#define V4L2_CAPTURE_H

#include <string>
#include <memory>
#include <atomic>
#include <format>
#include "Log.h"
#include "Define.h"
#include "ColorConvert.h"

extern "C"
{
#include <libavdevice/avdevice.h>
#include <libavformat/avformat.h>
#include <libavutil/dict.h>
#include <libavutil/error.h>
}

class V4l2Capture
{
public:
    V4l2Capture() = default;
    ~V4l2Capture();

    V4l2Capture(const V4l2Capture&) = delete;
    V4l2Capture& operator=(const V4l2Capture&) = delete;

    bool Config(const V4l2CaptureConfig& Config);

    // Read 1 frame in v4l2
    UniquePacketPtr ReadPacket();
    UniqueFramePtr ConvertPacketToFrame(UniquePacketPtr Packet);
    
    AVStream* GetStream();
    void Close();

private:
    bool FFmpegConfig();
    bool V4l2NativeConfig();
    UniquePacketPtr ReadPacketFromFFmpeg();
    UniquePacketPtr ReadPacketFromV4l2();

    // FFmpeg Zone
    AVFormatContext*         m_FormatContext = nullptr;
    V4l2CaptureConfig        m_Config;
    int                      m_VideoStreamIndex = -1;
    // end FFmpeg Zone
    
    // V4l2 Zone
    int                      m_V4l2NativeFD = -1;
    VideoBuffer              m_VideoBuffer;
    v4l2_buffer              m_V4l2Config;
    // end V4l2 Zone

};

#endif // V4L2_CAPTURE_H