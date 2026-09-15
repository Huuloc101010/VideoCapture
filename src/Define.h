#ifndef DEFINE_H
#define DEFINE_H

#include <string>

struct V4l2CaptureConfig
{
    std::string Device;
    int Width;
    int Heigh;
    int FPS;
};

#endif // DEFINE_H