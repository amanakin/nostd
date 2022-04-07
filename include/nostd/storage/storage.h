#pragma once

#include <cstddef>
#include <cassert>
#include <exception>
#include <stdexcept>
#include <new>
#include <initializer_list>
#include <algorithm>

#include <nostd/util.h>

// Storage:
// T& data(size_t idx);
// const T& data(size_t idx) const;
// size_t capacity() const;

namespace nostd::storage {

// ============================================================================

template <typename T, size_t Capacity>
struct LocalStorage {
    LocalStorage() = default;

    LocalStorage(const LocalStorage& other) noexcept;
    LocalStorage& operator=(const LocalStorage& other) noexcept;

    LocalStorage(LocalStorage&& other) noexcept;
    LocalStorage& operator=(LocalStorage&& other) noexcept;

    virtual const T& data(size_t idx) const;
    virtual T& data(size_t idx);

    [[nodiscard]] virtual size_t capacity() const;

    virtual ~LocalStorage() = default;

private:
    T data_[Capacity];
};

template <typename T, size_t Capacity>
LocalStorage<T, Capacity>::LocalStorage(const LocalStorage& other) noexcept {
    for (size_t idx = 0; idx < Capacity; ++idx) {
        data_[idx] = other.data_[idx];
    }
}

template <typename T, size_t Capacity>
LocalStorage<T, Capacity>& LocalStorage<T, Capacity>::operator=(const LocalStorage& other) noexcept {
    if (this == &other) {
        return *this;
    }

    for (size_t idx = 0; idx < Capacity; ++idx) {
        data_[idx] = other.data_[idx];
    }

    return *this;
}

template <typename T, size_t Capacity>
LocalStorage<T, Capacity>::LocalStorage(LocalStorage&& other) noexcept {
    *this = other;
}

template <typename T, size_t Capacity>
LocalStorage<T, Capacity>& LocalStorage<T, Capacity>::operator=(LocalStorage&& other) noexcept {
    *this = other;

    return *this;
}

template <typename T, size_t Size>
const T& LocalStorage<T, Size>::data(size_t idx) const {
    if (idx >= Size) {
        throw std::out_of_range("LocalStorage: data error");
    }
    return data_[idx];
}

template <typename T, size_t Size>
T& LocalStorage<T, Size>::data(size_t idx) {
    return const_cast<T&>(
           const_cast<const LocalStorage*>(this)->data(idx)
           );
}

template <typename T, size_t Size>
size_t LocalStorage<T, Size>::capacity() const {
    return Size;
}

// ============================================================================

template <typename T>
struct DynamicStorage {
    DynamicStorage() noexcept;
    explicit DynamicStorage(size_t capacity);

    virtual const T& data(size_t idx) const;
    virtual T& data(size_t idx);

    [[nodiscard]] virtual size_t capacity() const;

    void resize(size_t capacity);

    virtual ~DynamicStorage();

private:
    T* data_;
    size_t capacity_;
};

template <typename T>
DynamicStorage<T>::DynamicStorage() noexcept
    : data_(nullptr), capacity_(0) {
}

template <typename T>
DynamicStorage<T>::DynamicStorage(size_t capacity)
    : data_(nullptr), capacity_(capacity) {
    resize(capacity_);
}

template <typename T>
const T& DynamicStorage<T>::data(size_t idx) const {
    if (idx >= capacity_) {
        throw std::out_of_range("DynamicStorage: data error");
    }
    return data_[idx];
}

template <typename T>
T& DynamicStorage<T>::data(size_t idx) {
    return const_cast<T&>(
           const_cast<const DynamicStorage*>(this)->data(idx)
           );
}

template <typename T>
DynamicStorage<T>::~DynamicStorage() {
    delete[] data_;
}

template <typename T>
size_t DynamicStorage<T>::capacity() const {
    return capacity_;
}

template <typename T>
void DynamicStorage<T>::resize(size_t new_capacity) {
    auto new_data = new T[new_capacity];
    if (new_data == nullptr) {
        throw std::bad_alloc();
    }

    if (data_ != nullptr) {
        for (size_t idx = 0; idx < std::min(new_capacity, capacity_); ++idx) {
            new_data[idx] = data_[idx];
        }

        delete[] data_;
    }

    capacity_ = new_capacity;
    data_ = new_data;
}

} // nostd::storage

// ============================================================================
