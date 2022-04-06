#pragma once

#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <stdexcept>

#include "../util.h"

namespace nostd {

// ============================================================================

template <
    typename T,
    template <typename StorageType> typename Storage
    >
struct Array: public Storage<T> {
    Array() noexcept = default;
    explicit Array(size_t size);
    Array(size_t size, const T& val);
    Array(std::initializer_list<T> list);

    Array(const Array& other) = default;
    Array& operator=(const Array& other) = default;

    Array(Array&& other) noexcept = default;
    Array& operator=(Array&& other) noexcept = default;

    ~Array() = default;

    // access
    const T& operator[](size_t idx) const;
    T& operator[](size_t idx);

    T& front();
    const T& front() const;

    T& back();
    const T& back() const;

    // storage
    [[nodiscard]] bool empty() const noexcept;
    [[nodiscard]] size_t size() const noexcept;
    void resize(size_t size);
    void clear() noexcept;

    // modifiers
    template <typename... Args>
    void emplace_back(Args&&... args) {
        Storage<T>::emplace_back(std::forward<Args>(args)...);
    }

    void push_back(const T& value);
    void push_back(T&& value);

    void pop_back();
protected:
    void expand_capacity();

    size_t size_{0};

    using Storage<T>::data;
    using Storage<T>::capacity;
};

// ----------------------------------------------------------------------------

template <
        typename T,
        template <typename StorageType> typename Storage
        >
Array<T, Storage>::Array(size_t size)
    : size_(size), Storage<T>(size) {
}


template <
        typename T,
        template <typename StorageType> typename Storage
        >
Array<T, Storage>::Array(size_t size, const T& val)
    : size_(size), Storage<T>(size) {
    for (size_t idx = 0; idx < size; ++idx) {
        data[idx] = val;
    }
}


template <
        typename T,
        template <typename StorageType> typename Storage
        >
Array<T, Storage>::Array(std::initializer_list<T> list)
    : size_(list.size()), Storage<T>(list.size()) {
    for (size_t idx = 0; idx < size_; ++idx) {
        data(idx) = std::move(list[idx]);
    }
}

template <
        typename T,
        template <typename StorageType> typename Storage
        >
const T& Array<T, Storage>::operator[](size_t idx) const {
    return data(idx);
}

template <
        typename T,
        template <typename StorageType> typename Storage
        >
T& Array<T, Storage>::operator[](size_t idx) {
    return data(idx);
}

template <
        typename T,
        template <typename StorageType> typename Storage
        >
T& Array<T, Storage>::front() {
    return data(0);
}

template <
        typename T,
        template <typename StorageType> typename Storage
        >
const T& Array<T, Storage>::front() const {
    return data(0);
}

template <
        typename T,
        template <typename StorageType> typename Storage
        >
T& Array<T, Storage>::back() {
    return data(size_ - 1);
}

template <
        typename T,
        template <typename StorageType> typename Storage
        >
const T& Array<T, Storage>::back() const {
    return data(size_ - 1);
}

template <
        typename T,
        template <typename StorageType> typename Storage
        >
bool Array<T, Storage>::empty() const noexcept {
    return size_ == 0;
}

template <
        typename T,
        template <typename StorageType> typename Storage
        >
size_t Array<T, Storage>::size() const noexcept {
    return size_;
}

template <
        typename T,
        template <typename StorageType> typename Storage
        >
void Array<T, Storage>::resize(size_t size) {
    Storage<T>::resize(size);
}

template <
        typename T,
        template <typename StorageType> typename Storage
        >
void Array<T, Storage>::clear() noexcept {
    Storage<T>::resize(0);
}

template <
        typename T,
        template <typename StorageType> typename Storage
        >
void Array<T, Storage>::push_back(const T& value) {
    if (size_ == capacity()) {
        expand_capacity();
    }

    data(size_++) = std::move(value);
}

template <
        typename T,
        template <typename StorageType> typename Storage
        >
void Array<T, Storage>::push_back(T&& value) {
    if (size_ == capacity()) {
        expand_capacity();
    }

    data(size_++) = std::move(value);
}

template <
        typename T,
        template <typename StorageType> typename Storage
        >
void Array<T, Storage>::pop_back() {
    if (size_ == 0) {
        return;
    }

    size_--;
}

template <
        typename T,
        template <typename StorageType> typename Storage
>
void Array<T, Storage>::expand_capacity() {
    Storage<T>::resize(size_ * 2);
}

// ============================================================================
/*
namespace detail {
    inline constexpr size_t BoolChunkSize{8};
}

template <template <typename StorageType> typename Storage>
struct Array<bool, Storage> : public Storage<uint8_t> {
private:
    struct Reference {
        operator bool() const;

        Reference& operator=(const Reference& other);
        Reference& operator=(bool other);

        Reference(uint8_t& chunk, uint8_t offset);
    private:
        uint8_t* chunk_;
        uint8_t offset_;
    };

public:
    Array() noexcept = default;
    explicit Array(size_t size);
    Array(size_t size, bool val);
    Array(std::initializer_list<bool> list);

    Array(const Array& other) = default;
    Array& operator=(const Array& other) = default;

    Array(Array&& other) noexcept = default;
    Array& operator=(Array&& other) noexcept = default;

    ~Array() = default;

    // access
    const Reference operator[](size_t idx) const;
    Reference operator[](size_t idx);

    Reference front();
    const Reference front() const;

    Reference back();
    const Reference back() const;

    // storage
    bool empty() const noexcept;
    size_t size() const noexcept;
    void resize();
    void clear() noexcept;

    // modifiers
    template <typename... Args>
    void emplace_back(Args&&... args) {
        Storage<bool>::emplace_back(std::forward<Args>(args)...);
    }

    void push_back(bool value);

    void pop_back();
private:
    size_t count_;

    using Storage<uint8_t>::data;
    using Storage<uint8_t>::size;
};

// ============================================================================

template <template <typename StorageType> typename Storage>
Array<bool, Storage>::Array(size_t size)
    : Storage<bool>(util::CeilDiv(size, detail::BoolChunkSize)) {
}

template <template <typename StorageType> typename Storage>
Array<bool, Storage>::Array(size_t size, bool val)
    : Storage<bool>(util::CeilDiv(size, detail::BoolChunkSize), val) {
}

template <template <typename StorageType> typename Storage>
Array<bool, Storage>::Array(std::initializer_list<bool> list)
    : Storage<bool>(list) {
}

// ----------------------------------------------------------------------------

template <template <typename StorageType> typename Storage>
Array<bool, Storage>::Reference::operator bool() const {
    return (*chunk_) & (1 << offset_);
}

template <template <typename StorageType> typename Storage>
typename Array<bool, Storage>::Reference&
Array<bool, Storage>::Reference::operator=(const Reference& other) {
    return operator=(static_cast<bool>(other));
}

template <template <typename StorageType> typename Storage>
typename Array<bool, Storage>::Reference&
Array<bool, Storage>::Reference::operator=(bool other) {
    if (other) {
        *chunk_ |= 1 << offset_;
    } else {
        *chunk_ &= ~(1 << offset_);
    }

    return *this;
}

template <template <typename StorageType> typename Storage>
Array<bool, Storage>::Reference::Reference(uint8_t &chunk, uint8_t offset)
    : chunk_(&chunk), offset_(offset) {
}

// ----------------------------------------------------------------------------

template <template <typename StorageType> typename Storage>
const typename Array<bool, Storage>::Reference
Array<bool, Storage>::operator[](size_t idx) const {
    return Reference(data(idx >> 3), idx & 7);
}

template <template <typename StorageType> typename Storage>
typename Array<bool, Storage>::Reference
Array<bool, Storage>::operator[](size_t idx) {
    return Reference(data(idx >> 3), (idx & 7));
}

template <template <typename StorageType> typename Storage>
typename Array<bool, Storage>::Reference
Array<bool, Storage>::front() {
    return Reference(data(0), 0);
}

template <template <typename StorageType> typename Storage>
const typename Array<bool, Storage>::Reference
Array<bool, Storage>::front() const {
    return Reference(data(0), 0);
}

template <template <typename StorageType> typename Storage>
typename Array<bool, Storage>::Reference
Array<bool, Storage>::back() {
    return Reference(data(size()-1), count_ >> 3);
}

template <template <typename StorageType> typename Storage>
const typename Array<bool, Storage>::Reference
Array<bool, Storage>::back() const {
    return Reference(data(size()-1), count_ >> 3);
}

bool empty() const noexcept;
size_t size() const noexcept;
void resize();
void clear() noexcept;


template <template <typename StorageType> typename Storage>
bool Array<bool, Storage>::empty() const noexcept {
    return size == 0;
}*/


} // nostd

// ============================================================================
