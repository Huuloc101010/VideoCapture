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
    
    void Capture();
};

#endif // VIDEO_RECORDER_H