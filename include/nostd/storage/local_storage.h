#pragma once


namespace nostd::storage {

template <typename T, size_t Capacity>
struct LocalStorage {
    LocalStorage() = default;

    LocalStorage(const LocalStorage& other) noexcept;
    LocalStorage& operator=(const LocalStorage& other) noexcept;

    LocalStorage(LocalStorage&& other) noexcept;
    LocalStorage& operator=(LocalStorage&& other) noexcept;

    [[nodiscard]] virtual const T& data(size_t idx) const;
    [[nodiscard]] virtual T& data(size_t idx);

    [[nodiscard]] virtual size_t capacity() const;

    virtual ~LocalStorage() = default;

private:
    T data_[Capacity];
};

template <typename T, size_t Capacity>
LocalStorage<T, Capacity>::LocalStorage(const LocalStorage& other) noexcept {
    if (this == &other) {
        return;
    }

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
LocalStorage<T, Capacity>::LocalStorage(LocalStorage&& other) noexcept
    : LocalStorage(other) {
}

template <typename T, size_t Capacity>
LocalStorage<T, Capacity>& LocalStorage<T, Capacity>::operator=(LocalStorage&& other) noexcept {
    return this->operator=(other); // NOLINT
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

} // nostd::storage
