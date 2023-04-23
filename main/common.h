#pragma once
#include "algorithm"
#include "stdint.h"
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