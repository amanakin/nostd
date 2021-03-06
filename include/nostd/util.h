#pragma once

#include <type_traits>
#include <vector>
#include <concepts>

namespace nostd::util {

template<typename Int>
constexpr Int CeilDiv(const Int &lhs, const Int &rhs) {
    // static_assert(std::is_integral<Int>::value, "Only integral type required");
    return lhs / rhs + ((lhs % rhs) == 0 ? 0 : 1);
}

// ----------------------------------------------------------------------------

template <typename T, typename... Ts>
struct packSizeCounter {
    static constexpr size_t value = 1 + packSizeCounter<Ts...>::value;
};

template <typename T>
struct packSizeCounter<T> {
    static constexpr size_t value = 1;
};

template <typename... Ts>
constexpr size_t packSize = packSizeCounter<Ts...>::value;

} // nostd::util
