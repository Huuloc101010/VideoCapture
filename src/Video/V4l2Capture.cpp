#include "V4l2Capture.h"

V4l2Capture::~V4l2Capture()
{
    stop();
}

bool V4l2Capture::Config(const V4l2CaptureConfig& Config)
{
    PathDevice = Config.Device;
    is_running_.store(false);

    // Đăng ký toàn bộ thiết bị (FFmpeg mới có thể là no-op nhưng gọi để an toàn)
    avdevice_register_all();

    AVInputFormat* input_fmt = av_find_input_format("video4linux2");
    if (!input_fmt) {
        LOGE("Không tìm thấy v4l2 input format!");
        return false;
    }

    AVDictionary* opts = nullptr;
    std::string res_str = std::format("{}x{}", Config.Width, Config.Heigh);
    av_dict_set(&opts, "video_size", res_str.c_str(), 0);
    std::string fps_str = std::format("{}", Config.FPS);
    av_dict_set(&opts, "framerate", fps_str.c_str(), 0);
    // Có thể ép pixel format nếu muốn (vd: "yuyv422", "mjpeg", v.v.)
    // av_dict_set(&opts, "pixel_format", "yuyv422", 0);

    int ret = avformat_open_input(&fmt_ctx_, PathDevice.c_str(), input_fmt, &opts);
    av_dict_free(&opts);

    if (ret < 0) {
        char errbuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, errbuf, sizeof(errbuf));
        LOGE("Không thể mở thiết bị {}: {}", PathDevice, errbuf);
        return false;
    }

    if (avformat_find_stream_info(fmt_ctx_, nullptr) < 0) {
        LOGE("Không tìm thấy stream info cho {}", PathDevice);
        close();
        return false;
    }

    video_stream_index_ = -1;
    for (unsigned int i = 0; i < fmt_ctx_->nb_streams; ++i) {
        if (fmt_ctx_->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_index_ = i;
            break;
        }
    }

    if (video_stream_index_ == -1) {
        LOGE("Không tìm thấy video stream trên {}", PathDevice);
        close();
        return false;
    }

    LOGI("V4L2 khởi tạo thành công: {} ({}, {} fps)", PathDevice, res_str, Config.FPS);
    return true;
}

bool V4l2Capture::start()
{
    if (!fmt_ctx_)
    {
        LOGE("Chưa gọi init() hoặc init thất bại!");
        return false;
    }
    is_running_.store(true);
    return true;
}

bool V4l2Capture::read_packet(AVPacket** pkt)
{
    if (!is_running_.load() || !fmt_ctx_) return false;

    while (is_running_.load()) {
        int ret = av_read_frame(fmt_ctx_, *pkt);
        if (ret < 0) {
            if (ret == AVERROR_EOF) {
                LOGW("EOF từ thiết bị v4l2");
            } else if (ret != AVERROR(EAGAIN)) {
                char errbuf[AV_ERROR_MAX_STRING_SIZE];
                av_strerror(ret, errbuf, sizeof(errbuf));
                LOGE("Lỗi đọc frame v4l2: {}", errbuf);
            }
            return false;
        }

        if ((*pkt)->stream_index == video_stream_index_) {
            return true;
        }
        //av_packet_unref(pkt);
    }
    return false;
}

AVStream* V4l2Capture::get_stream()
{
    return (fmt_ctx_ && video_stream_index_ >= 0) ? fmt_ctx_->streams[video_stream_index_] : nullptr;
}

void V4l2Capture::close()
{
    if (fmt_ctx_)
    {
        avformat_close_input(&fmt_ctx_);
        fmt_ctx_ = nullptr;
    }
}

void V4l2Capture::stop()
{
    is_running_.store(false);
    close();
}