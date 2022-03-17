#pragma once

#include <cstddef>
#include <cstdint>

#include <util.h>

inline constexpr size_t BoolChunkSize{8};

// ============================================================================

template <
    typename T,
    size_t Size,
    template <typename StorageType, size_t StorageSize> typename Storage
    >
struct Array: public Storage<T, Size> {
    const T& operator[](size_t idx) const;
    T& operator[](size_t idx);

private:
    using Storage<T, Size>::data;
};

template <
        typename T,
        size_t Size,
        template <typename StorageType, size_t StorageSize> typename Storage
        >
const T& Array<T, Size, Storage>::operator[](size_t idx) const {
    return data(idx);
}

template <
        typename T,
        size_t Size,
        template <typename StorageType, size_t StorageSize> typename Storage
        >
T& Array<T, Size, Storage>::operator[](size_t idx) {
    return data(idx);
}

// ============================================================================

template <
    size_t Size,
    template <typename StorageType, size_t StorageSize> typename Storage
    >
struct Array<bool, Size, Storage>:
        public Storage<uint8_t, util::CeilDiv(Size, BoolChunkSize)> {

    bool operator[](size_t idx) const {

    }

    bool& operator[](size_t idx) {

    }
private:
    using Storage<bool, util::CeilDiv(Size, BoolChunkSize)>::data;
};

template <
        size_t Size,
        template <typename StorageType, size_t StorageSize> typename Storage
        >
bool operator[](size_t idx) const {

}