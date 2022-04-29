#pragma once

#include <nostd/storage/local_storage.h>
#include <nostd/storage/dynamic_storage.h>
#include <nostd/storage/dynamic_storage.h>


/*
 * Storage requirements:

 * Storage();

 * void allocate(size_t capacity);
 * void deallocate();

 * size_t capacity() const;

 * template<class... Args >
   void construct(size_t idx, Args&&... args );
 * void destruct(size_t idx);

 * void swap(const other& Storage);

 * const T& operator[](size_t) const;
 * T& operator[](size_t);
 */
