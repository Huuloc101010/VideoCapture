#include "V4l2Capture.h"
#include "Define.h"

V4l2Capture::~V4l2Capture()
{
    Stop();
}

bool V4l2Capture::Config(const V4l2CaptureConfig& Config)
{
    PathDevice = Config.Device;

    // Register with ffmpeg
    avdevice_register_all();

    AVInputFormat* InputFormat = av_find_input_format("video4linux2");
    if (InputFormat == nullptr)
    {
        LOGE("V4l2 input format not found");
        return false;
    }

    AVDictionary* Opts = nullptr;
    std::string res_str = std::format("{}x{}", Config.Width, Config.Heigh);
    av_dict_set(&Opts, "video_size", res_str.c_str(), 0);
    std::string fps_str = std::format("{}", Config.FPS);
    av_dict_set(&Opts, "framerate", fps_str.c_str(), 0);
    av_dict_set(&Opts, "pixel_format", "yuyv422", 0);

    int Retval = avformat_open_input(&FormatContext, PathDevice.c_str(), InputFormat, &Opts);
    av_dict_free(&Opts);

    if (Retval < 0)
    {
        char errbuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(Retval, errbuf, sizeof(errbuf));
        LOGE("Can not open device {}: {}", PathDevice, errbuf);
        return false;
    }

    if (avformat_find_stream_info(FormatContext, nullptr) < 0)
    {
        LOGE("Can not find stream info {}", PathDevice);
        Close();
        return false;
    }

    VideoStreamIndex = -1;
    for (unsigned int i = 0; i < FormatContext->nb_streams; ++i) {
        if (FormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            VideoStreamIndex = i;
            break;
        }
    }

    if (VideoStreamIndex == -1) {
        LOGE("Can not find video stream int path {}", PathDevice);
        Close();
        return false;
    }

    LOGI("V4L2 init success: {} ({}, {} fps)", PathDevice, res_str, Config.FPS);
    return true;
}

UniquePacketPtr V4l2Capture::ReadPacket()
{
    if (FormatContext == nullptr)
    {
        LOGE("FormatContext == nullptr");
        return {};
    }
    UniquePacketPtr Packet(av_packet_alloc());

    for(int i = 0; i < 5; ++i) // Try 5 time
    {
        int Retval = av_read_frame(FormatContext, Packet.get());
        if (Retval)
        {
            if(Retval == AVERROR_EOF)
            {
                LOGE("EOF v4l2");
                return {};
            }
            else if(Retval != AVERROR(EAGAIN))
            {
                char ErrBuffer[AV_ERROR_MAX_STRING_SIZE];
                av_strerror(Retval, ErrBuffer, sizeof(ErrBuffer));
                LOGW("Error read v4l2: {}, Trying {} time left", ErrBuffer, 5 - i);
                // Try again
            }
        }

        if (Packet->stream_index == VideoStreamIndex)
        {
            return std::move(Packet);
        }
    }
    return {};
}

AVStream* V4l2Capture::GetStream()
{
    return (FormatContext && VideoStreamIndex >= 0) ? FormatContext->streams[VideoStreamIndex] : nullptr;
}

void V4l2Capture::Close()
{
    if (FormatContext)
    {
        avformat_close_input(&FormatContext);
        FormatContext = nullptr;
    }
}

void V4l2Capture::Stop()
{
    Close();
}