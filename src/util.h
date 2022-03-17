#pragma once

#include <type_traits>

namespace util {

template<typename Int>
constexpr Int CeilDiv(const Int &lhs, const Int &rhs) {
    // static_assert(std::is_integral<Int>::value, "Only integral type required");
    return lhs / rhs + ((lhs % rhs) == 0 ? 0 : 1);
}

} // util