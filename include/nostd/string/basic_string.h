#pragma once

#include <memory>

#include <nostd/pointers/shared_ptr.h>

/*
 * Small
 * Medium
 * Large
 */

template <typename CharT, typename Allocator = std::allocator<CharT>>
class BaseString {
    using size_type = size_t;
    using value_type = CharT;
    using pointer = CharT*;
    using const_pointer = const CharT*;

    struct _Short {

    };

    struct _Medium {
        size_type capacity_;
        size_type size_;
        pointer data_;
    };

    struct _Long {
        std::shared_ptr<value_type> data_;
        size_type capacity_;
        size_type size_;
    };

    std::shared_ptr<int> a;

};