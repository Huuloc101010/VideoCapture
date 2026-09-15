#ifndef DEFINE_H
#define DEFINE_H

#include <string>
#include <memory>
extern "C"
{
    #include <libavutil/imgutils.h>
    #include <libavutil/samplefmt.h>
    #include <libavutil/timestamp.h>
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswresample/swresample.h>
}

struct V4l2CaptureConfig
{
    std::string Device;
    int Width;
    int Heigh;
    int FPS;
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

#endif // DEFINE_H