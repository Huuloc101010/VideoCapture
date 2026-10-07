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
    #include <libswscale/swscale.h>
    #include <libavutil/opt.h>
    #include <libavdevice/avdevice.h>
    #include <libavutil/avutil.h>
    #include <libavutil/channel_layout.h>
}

enum class CaptureType : uint8_t
{
    FFMPEG_CAPTURE,
    V4L2_NATIVE,
};

enum class MediaType : uint8_t
{
    VIDEO,
    AUDIO,
    SUBTITLE,
    OTHER,
};

struct V4l2CaptureConfig
{
    CaptureType Type;
    std::string Device;
    int Width;
    int Height;
    int FPS;
    AVPixelFormat PixelFormat;
};

struct AlsaCaptureConfig
{
    std::string Device;
    std::string SampleRate;
    std::string Channels;
};

struct ColorConvertConfig
{
    int Width;
    int Height;
    AVPixelFormat PixelFormat;
};

struct EncoderConfig
{
    int Width;
    int Height;
    MediaType Type;
};


// struct for mmap
struct VideoBuffer
{
    void* start{nullptr};
    size_t length{0};
};

struct TimeRecord
{
    int Hour;
    int Minutes;
    int Second;
};

struct VideoRecorderConfig
{
    std::string PathDevice;
    std::string PathVideoOutput;
    TimeRecord Time;
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
            Resource = nullptr;
        }
    }
};


// ffmpeg resource
using UniqueFramePtr      = std::unique_ptr<AVFrame, UniquePtrDeleterLevel2<AVFrame, av_frame_free>>;
using UniquePacketPtr     = std::unique_ptr<AVPacket, UniquePtrDeleterLevel2<AVPacket, av_packet_free>>;
using UniqueFormatContext = std::unique_ptr<AVFormatContext, UniquePtrDeleterLevel2<AVFormatContext, avformat_close_input>>;
using UniqueCodecContext  = std::unique_ptr<AVCodecContext, UniquePtrDeleterLevel2<AVCodecContext, avcodec_free_context>>;
using UniqueSwrContext    = std::unique_ptr<SwrContext, UniquePtrDeleterLevel2<SwrContext, swr_free>>;
using AVCodecPtr          = AVCodec *;
using AVStreamPtr         = AVStream *;
using AVCodecParPtr       = AVCodecParameters *;

struct MuxerConfig
{
    int Width;
    int Height;
    AVCodecContext* VideoCodecContex;
    AVCodecContext* AudioCodecContex;
    std::string Extension;
    std::string VideoName;
};

struct AudioConvertConfig
{
    AVStreamPtr AudioStream;
    int         SampleRate;      // 48000
    int         ChannelLayout;   // Mono or stereo
};

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