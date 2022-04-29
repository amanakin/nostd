#pragma once

namespace nostd::storage {

template <typename T, typename Allocator = std::allocator<T>>
struct DynamicStorage {
    using value_type = T;
    using size_type = size_t;

    explicit DynamicStorage(const Allocator& alloc = Allocator()) noexcept;
    void allocate(size_type cap);
    void deallocate();
    void swap(const DynamicStorage& other);

    // Data
    [[nodiscard]] size_type capacity() const;

    template <typename... Args>
    void construct(size_type idx, Args&&... args ) {
        traits_t::construct(alloc_, data_ + idx, std::forward<Args>(args)...);
    }
    void destruct(size_type idx);

    [[nodiscard]] const T& operator[](size_type idx) const;
    [[nodiscard]] T& operator[](size_type idx);

private:
    using traits_t = std::allocator_traits<Allocator>;

    Allocator alloc_;
    T* data_{nullptr};
    size_type capacity_{};
};

// ----------------------------------------------------------------------------

template <typename T, typename Allocator>
DynamicStorage<T, Allocator>::DynamicStorage(const Allocator& alloc) noexcept
    : alloc_(alloc) {
}

template <typename T, typename Allocator>
void DynamicStorage<T, Allocator>::allocate(size_type cap) {
    if (capacity_ == 0) {
        return;
    }

    data_ = traits_t::allocate(alloc_, cap);
    capacity_ = cap;
}

template <typename T, typename Allocator>
void DynamicStorage<T, Allocator>::deallocate() {
    if (capacity_ == 0) {
        return;
    }

    traits_t::deallocate(alloc_, data_, capacity_);
}

template <typename T, typename Allocator>
void DynamicStorage<T, Allocator>::swap(const DynamicStorage& other) {
    std::swap(capacity_, other.capacity_);
    std::swap(alloc_, other.alloc_);
    std::swap(data_, other.data_);
}

// ----------------------------------------------------------------------------

template <typename T, typename Allocator>
typename DynamicStorage<T, Allocator>::size_type DynamicStorage<T, Allocator>::capacity() const {
    return capacity_;
}

template <typename T, typename Allocator>
void DynamicStorage<T, Allocator>::destruct(size_type idx) {
    traits_t::destroy(alloc_, data_ + idx);
}

template <typename T, typename Allocator>
const typename DynamicStorage<T, Allocator>::value_type& DynamicStorage<T, Allocator>::operator[](size_type idx) const {

}

template <typename T, typename Allocator>
typename DynamicStorage<T, Allocator>::value_type& DynamicStorage<T, Allocator>::operator[](size_type idx) {

}

// ----------------------------------------------------------------------------

} // nostd::storage