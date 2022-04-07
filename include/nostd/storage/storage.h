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

template <typename T, size_t Capacity>
const T& LocalStorage<T, Capacity>::data(size_t idx) const {
    if (idx >= Capacity) {
        throw std::out_of_range("LocalStorage: data error");
    }
    return data_[idx];
}

template <typename T, size_t Capacity>
T& LocalStorage<T, Capacity>::data(size_t idx) {
    return const_cast<T&>(
           const_cast<const LocalStorage*>(this)->data(idx)
           );
}

template <typename T, size_t Capacity>
size_t LocalStorage<T, Capacity>::capacity() const {
    return Capacity;
}

// ============================================================================

template <typename T>
struct DynamicStorage {
    DynamicStorage() noexcept;
    explicit DynamicStorage(size_t capacity);

    DynamicStorage(const DynamicStorage& other);
    DynamicStorage& operator=(const DynamicStorage& other);

    DynamicStorage(DynamicStorage&& other) noexcept;
    DynamicStorage& operator=(DynamicStorage&& other) noexcept;

    virtual const T& data(size_t idx) const;
    virtual T& data(size_t idx);

    [[nodiscard]] virtual size_t capacity() const;

    // Allocate data and copy prev data
    void resize(size_t new_capacity);

    virtual ~DynamicStorage();
private:
    // Allocate empty data
    void allocate(size_t new_capacity);

    T* data_;
    size_t capacity_;
};

template <typename T>
DynamicStorage<T>::DynamicStorage() noexcept
    : data_(nullptr), capacity_(0) {
}

template <typename T>
DynamicStorage<T>::DynamicStorage(size_t capacity)
    : data_(nullptr), capacity_(0) {
    allocate(capacity);
}

template <typename T>
DynamicStorage<T>::DynamicStorage(const DynamicStorage<T> &other)
    : data_(nullptr), capacity_(0) {
    allocate(other.capacity_);

    for (size_t idx = 0; idx < capacity_; ++idx) {
        data_[idx] = other.data_[idx];
    }
}

template <typename T>
DynamicStorage<T>& DynamicStorage<T>::operator=(const DynamicStorage<T> &other) {
    if (this == &other) {
        return *this;
    }

    allocate(other.capacity_);
    for (size_t idx = 0; idx < capacity_; ++idx) {
        data_[idx] = other.data_[idx];
    }

    return *this;
}

template <typename T>
DynamicStorage<T>::DynamicStorage(DynamicStorage<T> &&other) noexcept
    : data_(other.data_), capacity_(other.capacity_) {
    other.data_ = nullptr;
    other.capacity_ = 0;
}

template <typename T>
DynamicStorage<T>& DynamicStorage<T>::operator=(DynamicStorage<T> &&other) noexcept {
    if (this == &other) {
        return *this;
    }

    delete []data_;
    data_ = other.data_;
    capacity_ = other.capacity_;

    other.data_ = nullptr;
    other.capacity_ = 0;

    return *this;
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
    DynamicStorage new_storage(new_capacity);

    for (size_t idx = 0; idx < std::min(capacity_, new_capacity); ++idx) {
        new_storage.data_[idx] = data_[idx];
    }

    this->operator=(std::move(new_storage));
}

template <typename T>
void DynamicStorage<T>::allocate(size_t new_capacity) {
    capacity_ = new_capacity;

    delete data_;
    data_ = new T[capacity_];
    if (data_ == nullptr) {
        throw std::bad_alloc();
    }
}

// ============================================================================



} // nostd::storage

