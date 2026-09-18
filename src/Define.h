#ifndef DEFINE_H
#define DEFINE_H

#include <string>
#include <memory>
#include <cstring>
extern "C"
{
    #include <libavutil/imgutils.h>
    #include <libavutil/samplefmt.h>
    #include <libavutil/timestamp.h>
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswresample/swresample.h>
}

enum class CaptureType : uint8_t
{
    FFMPEG_CAPTURE,
    V4L2_NATIVE,
};
struct V4l2CaptureConfig
{
    CaptureType Type;
    std::string Device;
    int Width;
    int Heigh;
    int FPS;
    AVPixelFormat PixelFormat;
};
//
// struct for mmap
struct VideoBuffer
{
    void* start{nullptr};
    size_t length{0};
};


template<typename T, void(*FreeFunction)(T*)>
class UniquePtrDeleterLevel1
{
public:
    void operator()(T* Resource)
    {
        if(Resource)
        {
            FreeFunction(Resource);
        }
    }
};

template<typename T, void(*FreeFunction)(T**)>
class UniquePtrDeleterLevel2
{
public:
    void operator()(T* Resource)
    {
        if(Resource)
        {
            FreeFunction(&Resource);
        }
    }
};


// ffmpeg resource
using UniqueFramePtr      = std::unique_ptr<AVFrame, UniquePtrDeleterLevel2<AVFrame, av_frame_free>>;
using UniquePacketPtr     = std::unique_ptr<AVPacket, UniquePtrDeleterLevel2<AVPacket, av_packet_free>>;
using UniqueFormatContext = std::unique_ptr<AVFormatContext, UniquePtrDeleterLevel2<AVFormatContext, avformat_close_input>>;

template<typename T>
class ScopeGuard
{
private:
    T Function;

public:
    explicit ScopeGuard(T Func) : Function(std::move(Func))
    {
    }

    ~ScopeGuard()
    {
        Function();
    }
};

#endif // DEFINE_H