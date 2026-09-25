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
#include "V4l2Capture.h"
#include "Encoder.h"
#include "Muxer.h"
#include "ColorConvert.h"

class VideoRecorder
{
public:
    VideoRecorder() = default;
    ~VideoRecorder() = default;
    
    void Record();
private:
    V4l2Capture       m_V4l2Capture;
    Encoder           m_Encoder;
    Muxer             m_Muxer;
    ColorConvert      m_ColorConvert;
};

#endif // VIDEO_RECORDER_H