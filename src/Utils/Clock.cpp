#include "Clock.h"

Clock::Clock()
{
    // Perform calculate time
    Start();
}

void Clock::Start()
{
    StartTime = std::chrono::steady_clock::now();
}

void Clock::Reset()
{
    // Reset clock
    Start();
}

double Clock::GetTimeSeconds()
{
    std::chrono::steady_clock::time_point EndTime = std::chrono::steady_clock::now();
    return std::chrono::duration<double>(EndTime - StartTime).count();
}

double Clock::GetTimeMiliSeconds()
{
    std::chrono::steady_clock::time_point EndTime = std::chrono::steady_clock::now();
    return std::chrono::duration<double, std::milli>(EndTime - StartTime).count();
}

double Clock::GetTimeMicroSeconds()
{
    std::chrono::steady_clock::time_point EndTime = std::chrono::steady_clock::now();
    return std::chrono::duration<double, std::micro>(EndTime - StartTime).count();
}