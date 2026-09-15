#ifndef V4L2_CAPTURE_H
#define V4L2_CAPTURE_H

#include <string>
#include <memory>
#include <atomic>
#include <format>
#include "Log.h"
#include "Define.h"

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
    
    void Stop();
    AVStream* GetStream();

private:
    void Close();
    
    std::string PathDevice;
    AVFormatContext* FormatContext = nullptr;
    int VideoStreamIndex = -1;
};

#endif // V4L2_CAPTURE_H