#include <vector>
#include <fstream>
#include <string>
#include "Utils.h"

Utils& Utils::GetInstance()
{
    static Utils Instance;
    return Instance;
}

bool Utils::SavePPM(const std::string& filename, const std::vector<uint8_t>& rgb, int width, int height)
{
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) return false;

    // Head of PPM format Binary RGB (P6)
    file << "P6\n" << width << " " << height << "\n255\n";

    // Write file
    file.write(reinterpret_cast<const char*>(rgb.data()), rgb.size());
    file.close();
    return true;
}
