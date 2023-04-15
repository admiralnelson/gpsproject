#pragma once
#include "algorithm"
template <typename T>
T clamp(T val, T min, T max) {
    return std::min(std::max(val, min), max);
}

template <typename T>
T map(T value, T fromLow, T fromHigh, T toLow, T toHigh) {
    return (value - fromLow) * (toHigh - toLow) / (fromHigh - fromLow) + toLow;
}