#ifndef VIDEO_RECORDER_H
#define VIDEO_RECORDER_H

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

class VideoRecorder
{
public:
    VideoRecorder() = default;
    ~VideoRecorder() = default;
    std::vector<uint8_t> convert_yuyv_to_rgb24(const uint8_t* yuyv, int width, int height);
    bool save_ppm(const std::string& filename, const std::vector<uint8_t>& rgb, int width, int height);
    void Capture();
};

#endif // VIDEO_RECORDER_H