#pragma once

namespace nostd::storage {

template <typename T, typename Allocator = std::allocator<T>>
struct AllocatorStorage {
    explicit AllocatorStorage(const Allocator& alloc = Allocator()) noexcept;
    explicit AllocatorStorage(size_t capacity, const Allocator& alloc = Allocator());

    AllocatorStorage(const AllocatorStorage& other);
    AllocatorStorage& operator=(const AllocatorStorage& other);

    AllocatorStorage(AllocatorStorage&& other) noexcept;
    AllocatorStorage& operator=(AllocatorStorage&& other) noexcept;

    virtual const T& data(size_t idx) const;
    virtual T& data(size_t idx);

    [[nodiscard]] virtual size_t capacity() const;

    // Allocate data and copy prev data
    void resize(size_t new_capacity);

    virtual ~AllocatorStorage();
private:
    // Allocate empty data
    void allocate(size_t new_capacity);

    void swap(AllocatorStorage& other);

    using traits_t = std::allocator_traits<Allocator>;

    Allocator alloc_;
    T* data_{nullptr};
    size_t capacity_{0};
};

template <typename T, typename Allocator>
AllocatorStorage<T, Allocator>::AllocatorStorage(const Allocator& alloc) noexcept
    : alloc_(alloc) {
}

template <typename T, typename Allocator>
AllocatorStorage<T, Allocator>::AllocatorStorage(size_t capacity, const Allocator& alloc)
    : alloc_(alloc) {

}

template <typename T, typename Allocator>
void AllocatorStorage<T, Allocator>::resize(size_t new_capacity) {
    auto new_data = traits_t::allocate(alloc_, new_capacity);
    for (size_t idx = 0; idx < new_capacity; ++idx) {
        traits_t::construct(alloc_, );
    }

}

template <typename T, typename Allocator>
void AllocatorStorage<T, Allocator>::allocate(size_t new_capacity) {
    if (capacity_ == 0) {
        return;
    }


}

template <typename T, typename Allocator>
void AllocatorStorage<T, Allocator>::deallocate() {
    if (capacity_ == 0) {
        return;
    }

    for (size_t idx = 0; idx < capacity_; ++idx) {
        traits_t::de
    }
}

} // nostd::storage