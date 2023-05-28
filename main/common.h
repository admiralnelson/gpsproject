#pragma once
#include "algorithm"
#include "stdint.h"
#include <string>
#include "cmath"
#include "queue"

#include <functional>
#include <chrono>
#include <memory>
#include <string>
#include "esp_timer.h"
#include "cjsoncpp.h"

typedef unsigned int uint;
typedef std::function<void()> VoidCallBack;

typedef tm TimeStructure;

void DisableBrownOutDetector(bool bDisableIt);

uint64_t inline Micros()
{
    uint64_t us = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::
        now().time_since_epoch()).count();
    return us;
}

time_t inline CurrentTime()
{
    time_t s = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::
        now().time_since_epoch()).count();
    return s;
}

uint64_t inline Uptime()
{
    return esp_timer_get_time();
}

std::string GenerateRandomString(size_t Length);

void PrintHex(const uint8_t* pData, uint32_t length);

template <typename Type>
void inline PrintLocalVariable(const Type& Data)
{
    PrintHex((const uint8_t*)&Data, sizeof(Type));
}


bool StringContainsChar(const std::string& A, char C);

//inclusive
int Clamp(int A, int Min, int Max);
//inclusive
bool IsBetween(int A, int Begin, int End);
std::string SubStringBeforeChar(std::string const& String, char A);

std::string TimeToString(time_t Time);

void PrintTime(const tm& Time);
void PrintTime(const time_t Time);
tm GetTmNow();

//will set the watchdog timer timeout to 0 and triggers cold reboot
void ColdReboot();


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


struct DateTime : JsonSerialisable
{
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;

    virtual cjsonpp::JSONObject ToJsonObject() const override;
    virtual bool FromJsonObject(const cjsonpp::JSONObject& Object) noexcept override;
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
