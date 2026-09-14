#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <vector>
#include <stdint.h>

class Utils
{
public:
    static Utils& GetInstance();
    bool SavePPM(const std::string& filename, const std::vector<uint8_t>& rgb, int width, int height);
};

#endif // UTILS_H
