#ifndef V4L2_CAPTURE_H
#define V4L2_CAPTURE_H

#include <string>
#include <memory>
#include <atomic>
#include <format>
#include "Log.h"

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

    bool Start(const std::string& device, const int width, const int height, const int fps);

    bool start();

    // Đọc frame (block cho đến khi có packet video tiếp theo)
    bool read_packet(AVPacket** pkt);
    

    void stop();
    AVStream* get_stream();

private:
    void close();
    

    std::string PathDevice;
    AVFormatContext* fmt_ctx_ = nullptr;
    int video_stream_index_ = -1;
    std::atomic<bool> is_running_{false};
};

#endif // V4L2_CAPTURE_H