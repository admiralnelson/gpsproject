#pragma once
#include "algorithm"
template <typename T>
T clamp(T val, T min, T max) {
    return std::min(std::max(val, min), max);
}