#pragma once
#include "algorithm"
#include "stdint.h"
#include <string>
#include "cmath"
#include "queue"

struct Vector3
{
    double x = 0;
    double y = 0;
    double z = 0;
};

template <typename T>
struct MinMax {
    T Min = 0;
    T Max = 0;
};


struct DateTime
{
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
};

template <typename T>
T clamp(T val, T min, T max) {
    return std::min(std::max(val, min), max);
}

template <typename T>
T map(T value, T fromLow, T fromHigh, T toLow, T toHigh) {
    return (value - fromLow) * (toHigh - toLow) / (fromHigh - fromLow) + toLow;
}

template<typename _Tp>
constexpr bool
IsBetween(const _Tp& __val, const _Tp& __lo, const _Tp& __hi)
{
    return __lo >= __val && __val <= __hi;
}


inline double rad2deg(double raddian)
{
    return raddian * 180.0 / M_PI;
}

inline double deg2rad(double deg)
{
    return deg * M_PI / 180.0;
}


uint64_t GetSystemMilliseconds();
void Delay(uint64_t howLongInMilliseconds);

bool SetDateAndTime(const std::string& dateTimeString);
long GetCurrentSystemDateTime();
DateTime GetCurrentSystemDateTimeStruct();
std::string GetCurrentSystemDateTimeAsString();

class MovingAverageFilter {
public:
    MovingAverageFilter(int windowSize) : windowSize(windowSize) {}

    double Filter(double value)
    {
        if (values.size() >= windowSize)
        {
            sum -= values.front();
            values.pop();
        }
        values.push(value);
        sum += value;
        return sum / values.size();
    }

private:
    int windowSize;
    double sum = 0;
    std::queue<double> values;
};
