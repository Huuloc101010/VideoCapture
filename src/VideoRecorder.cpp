#include "VideoRecorder.h"
#include "Log.h"
#include "Utils.h"
#include "Define.h"


// Function to convert YUYV to RGB24
std::vector<uint8_t> VideoRecorder::convert_yuyv_to_rgb24(const uint8_t* yuyv, int width, int height) {
    std::vector<uint8_t> rgb(width * height * 3);
    
    auto clamp = [](int val) -> uint8_t {
        return static_cast<uint8_t>(std::clamp(val, 0, 255));
    };

    size_t yuyv_idx = 0;
    size_t rgb_idx = 0;

    for (int i = 0; i < (width * height) / 2; ++i) {
        int y0 = yuyv[yuyv_idx++];
        int u  = yuyv[yuyv_idx++] - 128;
        int y1 = yuyv[yuyv_idx++];
        int v  = yuyv[yuyv_idx++] - 128;

        // cal YUV BT.601 -> RGB
        int c0 = y0 - 16;
        int c1 = y1 - 16;

        // Pixel 1
        rgb[rgb_idx++] = clamp((298 * c0 + 409 * v + 128) >> 8);           // R
        rgb[rgb_idx++] = clamp((298 * c0 - 100 * u - 208 * v + 128) >> 8);   // G
        rgb[rgb_idx++] = clamp((298 * c0 + 516 * u + 128) >> 8);           // B

        // Pixel 2
        rgb[rgb_idx++] = clamp((298 * c1 + 409 * v + 128) >> 8);           // R
        rgb[rgb_idx++] = clamp((298 * c1 - 100 * u - 208 * v + 128) >> 8);   // G
        rgb[rgb_idx++] = clamp((298 * c1 + 516 * u + 128) >> 8);           // B
    }

    return rgb;
}

void VideoRecorder::Capture()
{
    constexpr std::string_view dev_name = "/dev/video0";
    constexpr int width = 640;
    constexpr int height = 480;
    LOGI("test LOGI() {}", std::string("abc"), 123, "1111");
    LOGW("test LOGI() {}", std::string("abc"), 123, "1111");
    LOGE("test LOGI() {}", std::string("abc"), 123, "1111");

    // 1. Open v4l2
    int fd = open(dev_name.data(), O_RDWR);
    if (fd == -1) {
        std::cerr << "Can not open device" << dev_name << "\n";
        return;
    }

    // RAII cleanning file descriptor
    auto fd_cleanup = std::unique_ptr<int, decltype([](int* f) { if (f && *f >= 0) close(*f); })>(&fd);

    // 2. Setup format YUYV 640x480
    v4l2_format fmt{};
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = width;
    fmt.fmt.pix.height = height;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
    fmt.fmt.pix.field = V4L2_FIELD_NONE;

    if (ioctl(fd, VIDIOC_S_FMT, &fmt) < 0) {
        std::cerr << "Can not setup format YUYV\n";
        return;
    }

    // 3. Request Buffer mmap
    v4l2_requestbuffers req{};
    req.count = 1;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if (ioctl(fd, VIDIOC_REQBUFS, &req) < 0) {
        std::cerr << "Error VIDIOC_REQBUFS\n";
        return;
    }

    // 4. Mmap user space memory to kernel space for zero copy
    v4l2_buffer buf{};
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = 0;

    if (ioctl(fd, VIDIOC_QUERYBUF, &buf) < 0) {
        std::cerr << "Lỗi VIDIOC_QUERYBUF\n";
        return;
    }

    VideoBuffer video_buf;
    video_buf.length = buf.length;
    video_buf.start = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);

    if (video_buf.start == MAP_FAILED) {
        std::cerr << "Error mmap\n";
        return;
    }

    // RAII cleaning mmap
    auto mmap_cleanup = std::unique_ptr<VideoBuffer, decltype([](VideoBuffer* b) { 
        if (b && b->start) munmap(b->start, b->length); 
    })>(&video_buf);

    // 5. Put buffer into line wating & open Video Streaming mode
    if (ioctl(fd, VIDIOC_QBUF, &buf) < 0) return;

    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(fd, VIDIOC_STREAMON, &type) < 0) {
        std::cerr << "Error STREAMON\n";
        return;
    }

    // 6. Get 1 Frame from buffer mmap to process (Dequeue Buffer)
    if (ioctl(fd, VIDIOC_DQBUF, &buf) < 0) {
        std::cerr << "Error VIDIOC_DQBUF\n";
        return;
    }

    LOGI("Catured, capacity {} byte", buf.bytesused);

    // 7. Convert raw YUYV from mmap pointer to RGB24
    auto rgb_data = convert_yuyv_to_rgb24(static_cast<const uint8_t*>(video_buf.start), width, height);

    // 8. Close pipeline Streaming
    ioctl(fd, VIDIOC_STREAMOFF, &type);

    // 9. Save RGB24 intp ppm
    if(Utils::GetInstance().SavePPM("capture.ppm", rgb_data, width, height))
    {
        std::cout << "Saved file ppm success: capture.ppm\n";
    }
    else
    {
        std::cerr << "Error write file PPM!\n";
        return;
    }
}