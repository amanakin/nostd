#pragma once

#include <type_traits>

namespace nostd {

template <typename T>
    concept move_constructible =
        std::is_move_constructible_v<T>;

template <typename T>
    concept copy_constructible =
        std::is_copy_constructible_v<T>;

template <typename T>
    concept only_copy_constructible =
        copy_constructible<T> &&
        !move_constructible<T>;

} // nostd