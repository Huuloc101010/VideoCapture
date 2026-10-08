#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/videodev2.h>
#include "V4l2Capture.h"
#include "Define.h"
#include "Utils.h"

V4l2Capture::~V4l2Capture()
{
    Close();
}

bool V4l2Capture::Config(const V4l2CaptureConfig& Config)
{
    m_Config = Config;
    if(m_Config.Type == CaptureType::FFMPEG_CAPTURE)
    {
        return FFmpegConfig();
    }
    if(m_Config.Type == CaptureType::V4L2_NATIVE)
    {
        return V4l2NativeConfig();
    }

    LOGE("Not support this m_Config");
    return false;
}

bool V4l2Capture::FFmpegConfig()
{
    // Register with ffmpeg
    avdevice_register_all();

    const AVInputFormat* InputFormat = av_find_input_format("video4linux2");
    if (InputFormat == nullptr)
    {
        LOGE("V4l2 input format not found");
        return false;
    }

    AVDictionary* Opts = nullptr;
    ScopeGuard Guard([&]()
    {
        if(Opts != nullptr)
        {
            av_dict_free(&Opts);
        }
    });
    std::string res_str = std::format("{}x{}", m_Config.Width, m_Config.Height);
    av_dict_set(&Opts, "video_size", res_str.c_str(), 0);
    std::string fps_str = std::format("{}", m_Config.FPS);
    av_dict_set(&Opts, "framerate", fps_str.c_str(), 0);
    av_dict_set(&Opts, "pixel_format", "yuyv422", 0);
    AVFormatContext* FormatContext = nullptr;
    int Retval = avformat_open_input(&FormatContext, m_Config.Device.c_str(), const_cast<AVInputFormat*>(InputFormat), &Opts);
    m_FormatContext.reset(FormatContext);
    
    if(Retval < 0)
    {
        char errbuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(Retval, errbuf, sizeof(errbuf));
        LOGE("Can not open device {}: {}", m_Config.Device, errbuf);
        return false;
    }
    if(m_FormatContext == nullptr)
    {
        LOGE("Format context is null");
        return false;
    }

    if(m_FormatContext == nullptr)
    {
        LOGE("m_FormatContext == nullptr");
        return false;
    }

    if (avformat_find_stream_info(m_FormatContext.get(), nullptr) < 0)
    {
        LOGE("Can not find stream info {}", m_Config.Device);
        Close();
        return false;
    }

    m_VideoStreamIndex = -1;
    for (int i = 0; i < m_FormatContext->nb_streams; ++i)
    {
        if (m_FormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            m_VideoStreamIndex = i;
            break;
        }
    }

    if (m_VideoStreamIndex < 0)
    {
        LOGE("Can not find video stream int path {}", m_Config.Device);
        Close();
        return false;
    }
    AVStream* Stream = m_FormatContext->streams[m_VideoStreamIndex];

    LOGI(
        "Actual stream: codec={}, resolution={}x{}, time_base={}/{}, avg_frame_rate={}/{}, r_frame_rate={}/{}",
        "",
        Stream->codecpar->width,
        Stream->codecpar->height,
        Stream->time_base.num,
        Stream->time_base.den,
        Stream->avg_frame_rate.num,
        Stream->avg_frame_rate.den,
        Stream->r_frame_rate.num,
        Stream->r_frame_rate.den
    );

    LOGI("V4L2 init success: {} ({}, {} fps)", m_Config.Device, res_str, m_Config.FPS);
    AVCodecParPtr CodecPar = Stream->codecpar;
    LOGI("Video sample format: {}", Utils::GetInstance().GetSampleFormatName(MediaType::VIDEO , CodecPar->format));
    return true;
}

bool V4l2Capture::V4l2NativeConfig()
{
    //  Open v4l2
    m_V4l2NativeFD = open(m_Config.Device.data(), O_RDWR);
    if(m_V4l2NativeFD < 0)
    {
        LOGE("Can not open device in path {}", m_Config.Device);
        return false;
    }

    // Setup format
    v4l2_format fmt{};
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = m_Config.Width;
    fmt.fmt.pix.height = m_Config.Height;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
    fmt.fmt.pix.field = V4L2_FIELD_NONE;

    if(ioctl(m_V4l2NativeFD, VIDIOC_S_FMT, &fmt) < 0)
    {
        LOGE("VIDIOC_S_FMT failed: {}", strerror(errno));
        return false;
    }

    // Request Buffer mmap
    v4l2_requestbuffers req{};
    req.count = 1;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if(ioctl(m_V4l2NativeFD, VIDIOC_REQBUFS, &req) < 0)
    {
        LOGE("Error VIDIOC_REQBUFS");
        return {};
    }

    // Mmap user space memory to kernel space for zero copy
    m_V4l2Config.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    m_V4l2Config.memory = V4L2_MEMORY_MMAP;
    m_V4l2Config.index = 0;

    if(ioctl(m_V4l2NativeFD, VIDIOC_QUERYBUF, &m_V4l2Config) < 0)
    {
        LOGE("VIDIOC_QUERYBUF fail");
        return false;
    }

    m_VideoBuffer.length = m_V4l2Config.length;
    m_VideoBuffer.start = mmap(NULL, m_V4l2Config.length, PROT_READ | PROT_WRITE, MAP_SHARED, m_V4l2NativeFD, m_V4l2Config.m.offset);

    if (m_VideoBuffer.start == MAP_FAILED)
    {
        LOGE("Error when map memory");
        return false;
    }
    // Put buffer into line wating & open Video Streaming mode
    if(ioctl(m_V4l2NativeFD, VIDIOC_QBUF, &m_V4l2Config) < 0)
    {
        LOGE("VIDIOC_QBUF fail");
        return false;
    }

    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(ioctl(m_V4l2NativeFD, VIDIOC_STREAMON, &type) < 0)
    {
        LOGE("Error STREAMON");
        return {};
    }
    LOGI("Config success");
    return true;
}

UniquePacketPtr V4l2Capture::ReadPacket()
{
    if(m_Config.Type == CaptureType::V4L2_NATIVE)
    {
        return ReadPacketFromV4l2();
    }
    if(m_Config.Type == CaptureType::FFMPEG_CAPTURE)
    {
        return ReadPacketFromFFmpeg();
    }

    LOGE("V4l2 Capture m_Config type invalid");
    return {};
}

UniquePacketPtr V4l2Capture::ReadPacketFromFFmpeg()
{
    UniquePacketPtr Packet(av_packet_alloc());

    for(int i = 0; i < 5; ++i) // Try 5 time
    {
        int Retval = av_read_frame(m_FormatContext.get(), Packet.get());
        // LOGE("TimeBase: {}/{}",  GetStream()->time_base.num, GetStream()->time_base.den);
        // LOGE("ReadPacket Timebase {}", Utils::GetInstance().GetTimeStamp(GetStream()->time_base, Packet->pts));
        // LOGE("Packet pts {}", Packet->pts);
        if(Retval)
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
                continue;
            }
        }

        if (Packet->stream_index == m_VideoStreamIndex)
        {
            return std::move(Packet);
        }
    }
    return {};
}

UniquePacketPtr V4l2Capture::ReadPacketFromV4l2()
{
    if(m_V4l2NativeFD < 0)
    {
        LOGE("m_V4l2NativeFD invalid");
        return {};
    }
    if(m_VideoBuffer.start == nullptr)
    {
        LOGE("m_VideoBuffer.start == nullptr");
        return {};
    }
    // Get 1 Frame from buffer mmap to process (Dequeue Buffer)
    if (ioctl(m_V4l2NativeFD, VIDIOC_DQBUF, &m_V4l2Config) < 0)
    {
        LOGE("Error VIDIOC_DQBUF");
        return {};
    }
    ScopeGuard GuardMemory([&]()
    {
        // Return buffer
        if(ioctl(m_V4l2NativeFD, VIDIOC_QBUF, &m_V4l2Config) < 0)
        {
            LOGE("Error VIDIOC_DQBUF");
        }
    });

    LOGI("Catured , capacity: {} byte", m_V4l2Config.bytesused);
    UniquePacketPtr Packet(av_packet_alloc());
    int Ret = av_new_packet(Packet.get(), m_V4l2Config.bytesused);
    if(Ret < 0)
    {
        LOGE("av_new_packet() fail");
        return {};
    }
    std::memcpy(Packet->data, m_VideoBuffer.start, m_V4l2Config.bytesused);

    return Packet;
}

AVStreamPtr V4l2Capture::GetStream()
{
    return (m_FormatContext && m_VideoStreamIndex >= 0) ? m_FormatContext->streams[m_VideoStreamIndex] : nullptr;
}

void V4l2Capture::Close()
{
    if(m_VideoBuffer.start)
    {
        munmap(m_VideoBuffer.start, m_VideoBuffer.length);
        m_VideoBuffer.start = nullptr;
    }
    if(m_V4l2NativeFD >= 0)
    {
        close(m_V4l2NativeFD);
        enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        // Close pipeline Streaming
        ioctl(m_V4l2NativeFD, VIDIOC_STREAMOFF, &type);
        m_V4l2NativeFD = -1;
    }
}
