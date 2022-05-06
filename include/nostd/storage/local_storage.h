#pragma once


namespace nostd::storage {

template <typename T, size_t Capacity>
struct LocalStorageImpl {
    using value_type = T;
    using size_type = size_t;
    static constexpr auto value_size = sizeof(T);

    LocalStorageImpl() = default;
    void allocate(size_type cap);
    void deallocate();
    void swap(LocalStorageImpl& other);

    [[nodiscard]] size_type capacity() const;

    template <typename... Args>
    void construct(size_type idx, Args&&... args ) {
        new(data_ + idx * value_size) T(std::forward<Args>(args)...);
    }
    void destruct(size_type idx);

    [[nodiscard]] const T& operator[](size_type idx) const;
    [[nodiscard]] T& operator[](size_type idx);
private:
    alignas(sizeof(T)) char data_[Capacity * sizeof(T)];
};

template <typename T, size_t Capacity>
void LocalStorageImpl<T, Capacity>::allocate(size_type cap) {
}

template <typename T, size_t Capacity>
void LocalStorageImpl<T, Capacity>::deallocate() {
}

template <typename T, size_t Capacity>
void LocalStorageImpl<T, Capacity>::swap(LocalStorageImpl& other) {
    // TODO add swapping
}

// ----------------------------------------------------------------------------

template <typename T, size_t Capacity>
typename LocalStorageImpl<T, Capacity>::size_type LocalStorageImpl<T, Capacity>::capacity() const {
    return Capacity;
}

template <typename T, size_t Capacity>
void LocalStorageImpl<T, Capacity>::destruct(size_type idx) {
    reinterpret_cast<T*>(data_ + idx * value_size)->~T();
}

template <typename T, size_t Capacity>
const typename LocalStorageImpl<T, Capacity>::value_type& LocalStorageImpl<T, Capacity>::operator[](size_type idx) const {
    return *reinterpret_cast<const T*>(data_ + idx * value_size);
}
template <typename T, size_t Capacity>
typename LocalStorageImpl<T, Capacity>::value_type& LocalStorageImpl<T, Capacity>::operator[](size_type idx) {
    return *reinterpret_cast<T*>(data_ + idx * value_size);
}

} // nostd::storage
