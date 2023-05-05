#pragma once
#include "algorithm"
#include "stdint.h"
#include <string>


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

uint64_t GetSystemMilliseconds();
void Delay(uint64_t howLongInMilliseconds);

bool SetDateAndTime(const std::string& dateTimeString);
long GetCurrentSystemDateTime();
DateTime GetCurrentSystemDateTimeStruct();
std::string GetCurrentSystemDateTimeAsString();
