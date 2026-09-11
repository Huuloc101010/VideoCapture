#include <iostream>
#include <vector>
#include <string_view>
#include <memory>
#include <algorithm>
#include <fstream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/videodev2.h>

// struct for mmap
struct VideoBuffer {
    void* start{nullptr};
    size_t length{0};
};

// Function to convert YUYV to RGB24
std::vector<uint8_t> convert_yuyv_to_rgb24(const uint8_t* yuyv, int width, int height) {
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

// RGB24 -> PPM
bool save_ppm(const std::string& filename, const std::vector<uint8_t>& rgb, int width, int height) {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) return false;

    // Head of PPM format Binary RGB (P6)
    file << "P6\n" << width << " " << height << "\n255\n";

    // Write file
    file.write(reinterpret_cast<const char*>(rgb.data()), rgb.size());
    file.close();
    return true;
}

int main() {
    constexpr std::string_view dev_name = "/dev/video0";
    constexpr int width = 640;
    constexpr int height = 480;

    // 1. Open v4l2
    int fd = open(dev_name.data(), O_RDWR);
    if (fd == -1) {
        std::cerr << "Can not open device" << dev_name << "\n";
        return 1;
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
        return 1;
    }

    // 3. Request Buffer mmap
    v4l2_requestbuffers req{};
    req.count = 1;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if (ioctl(fd, VIDIOC_REQBUFS, &req) < 0) {
        std::cerr << "Error VIDIOC_REQBUFS\n";
        return 1;
    }

    // 4. Mmap user space memory to kernel space for zero copy
    v4l2_buffer buf{};
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = 0;

    if (ioctl(fd, VIDIOC_QUERYBUF, &buf) < 0) {
        std::cerr << "Lỗi VIDIOC_QUERYBUF\n";
        return 1;
    }

    VideoBuffer video_buf;
    video_buf.length = buf.length;
    video_buf.start = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);

    if (video_buf.start == MAP_FAILED) {
        std::cerr << "Error mmap\n";
        return 1;
    }

    // RAII cleaning mmap
    auto mmap_cleanup = std::unique_ptr<VideoBuffer, decltype([](VideoBuffer* b) { 
        if (b && b->start) munmap(b->start, b->length); 
    })>(&video_buf);

    // 5. Put buffer into line wating & open Video Streaming mode
    if (ioctl(fd, VIDIOC_QBUF, &buf) < 0) return 1;

    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(fd, VIDIOC_STREAMON, &type) < 0) {
        std::cerr << "Error STREAMON\n";
        return 1;
    }

    // 6. Get 1 Frame from buffer mmap to process (Dequeue Buffer)
    if (ioctl(fd, VIDIOC_DQBUF, &buf) < 0) {
        std::cerr << "Error VIDIOC_DQBUF\n";
        return 1;
    }

    std::cout << "Catured , capacity: " << buf.bytesused << " bytes.\n";

    // 7. Convert raw YUYV from mmap pointer to RGB24
    auto rgb_data = convert_yuyv_to_rgb24(static_cast<const uint8_t*>(video_buf.start), width, height);

    // 8. Close pipeline Streaming
    ioctl(fd, VIDIOC_STREAMOFF, &type);

    // 9. Save RGB24 intp ppm
    if (save_ppm("capture.ppm", rgb_data, width, height)) {
        std::cout << "Saved file ppm success: capture.ppm\n";
    } else {
        std::cerr << "Errir write file PPM!\n";
        return 1;
    }

    return 0;
}