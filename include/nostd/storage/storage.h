#pragma once

#include <nostd/storage/local_storage.h>
#include <nostd/storage/dynamic_storage.h>

namespace nostd::storage {

template <typename Allocator>
struct AllocatorStorage {
    template <typename T>
    using storage_type = DynamicStorageImpl<T, Allocator>;
};

template <typename T>
using DynamicStorage = DynamicStorageImpl<T>;

template <size_t Capacity>
struct LocalStorage {
    template <typename T>
    using storage_type = LocalStorageImpl<T, Capacity>;
};

} // nostd::storage

/*
 * Storage requirements:

 * Storage();

 * void allocate(size_t capacity);
 * void deallocate();

 * size_t capacity() const;

 * template<class... Args >
   void construct(size_t idx, Args&&... args );
 * void destruct(size_t idx);

 * void swap(other& Storage);

 * const T& operator[](size_t) const;
 * T& operator[](size_t);
 */
