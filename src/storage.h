#pragma once

#include <cstddef>
#include <cassert>
#include <exception>
#include <stdexcept>
#include <new>

// Storage:
// T& data(size_t idx);
// const T& data(size_t idx) const;

template <typename T, size_t Size>
struct LocalStorage {
    virtual const T& data(size_t idx) const;
    virtual T& data(size_t idx);

    virtual ~LocalStorage() = default;

private:
    T data_[Size];
};

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

// ============================================================================

template <typename T, size_t Size>
struct DynamicStorage {
    DynamicStorage();

    virtual const T& data(size_t idx) const;
    virtual T& data(size_t idx);

    virtual ~DynamicStorage();

private:
    T* data_;
};

template <typename T, size_t Size>
DynamicStorage<T, Size>::DynamicStorage() : data_(nullptr) {
    data_ = new T[Size];
    if (data_ == nullptr) {
        throw std::bad_alloc();
    }
}

template <typename T, size_t Size>
const T& DynamicStorage<T, Size>::data(size_t idx) const {
    if (idx >= Size) {
        throw std::out_of_range("DynamicStorage: data error");
    }
    return data_[idx];
}

template <typename T, size_t Size>
T& DynamicStorage<T, Size>::data(size_t idx) {
    return const_cast<T&>(
           const_cast<const DynamicStorage*>(this)->data(idx)
           );
}

template <typename T, size_t Size>
DynamicStorage<T, Size>::~DynamicStorage() {
    delete[] data_;
}

// ============================================================================
