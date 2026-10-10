#ifndef CLOCK_H
#define CLOCK_H

#include <chrono>

class Clock
{
public:
    Clock();
    void Start();
    void Reset();
    double GetTimeSeconds();
    double GetTimeMiliSeconds();
    double GetTimeMicroSeconds();

private:
    std::chrono::steady_clock::time_point StartTime;
};

#endif // CLOCK_H
