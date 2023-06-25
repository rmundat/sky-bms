#pragma once

#include <utility>
#include <cstdint>
#include <algorithm>
#include <cmath>

// Divide by zero protection
template<typename T1, typename T2>
inline auto safeDivide(T1 numerator, T2 denominator, T1 defaultInt = 0, T1 defaultFloat = 0.0) -> decltype(numerator / denominator) {
    static_assert(std::is_arithmetic<T1>::value && std::is_arithmetic<T2>::value, "safeDivide requires numeric types");

    if (std::is_integral<T1>::value && std::is_integral<T2>::value) {
        if (denominator == 0) {
            return defaultInt;
        }
    } else {
        if (denominator < std::numeric_limits<T2>::epsilon()) {
            return defaultFloat;
        }
    }
    return numerator / denominator;
}
