#pragma once

#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <stdexcept>
#include <concepts>

#include <nostd/concepts/concepts.h>
#include <nostd/storage/storage.h>

/*
 * reserve <-------------
 * shrink_to_fit        |
 * push_back_expand  ---|
 */

namespace nostd {

// ============================================================================

template <typename T, template<typename StorageT> typename Storage>
struct Array {

    template <bool isConst>
    class ArrayIterator {
    public:
        using difference_type = int64_t;
        using iterator_category = std::random_access_iterator_tag;

        using value_type = std::conditional_t<isConst, const T, T>;
        using pointer    = std::conditional_t<isConst, const T*, T*>;
        using reference  = std::conditional_t<isConst, const T&, T&>;

        bool operator==(const ArrayIterator& other) const {return index_ == other.index_ && array_ == other.array_;}
        bool operator!=(const ArrayIterator& other) const {return index_ != other.index_ || array_ != other.array_;}

        reference operator*()  const {return (*array_)[index_];}
        pointer   operator->() const {return array_ + index_;}

        ArrayIterator& operator++() noexcept {++index_; return *this;}
        ArrayIterator operator++(int) noexcept {
            ArrayIterator prev(*this);
            this->operator++();
            return prev;
        }

        ArrayIterator& operator--() noexcept {--index_; return *this;}
        ArrayIterator operator--(int) noexcept {
            ArrayIterator prev(*this);
            this->operator--();
            return prev;
        }

        ArrayIterator& operator+=(difference_type diff) noexcept {index_ += diff; return *this;}
        ArrayIterator& operator-=(difference_type diff) noexcept {index_ -= diff; return *this;}

        ArrayIterator operator+(difference_type diff) const noexcept { return ArrayIterator(index_ + diff, array_); }
        ArrayIterator operator-(difference_type diff) const noexcept { return ArrayIterator(index_ - diff, array_); }

        difference_type operator-(ArrayIterator& other) const {
            verify_array(other);
            return index_ - other.index_;
        }

        bool operator> (const  ArrayIterator& other) const {verify_array(other); return index_ > other.index_;}
        bool operator< (const  ArrayIterator& other) const {verify_array(other); return index_ < other.index_;}
        bool operator>=(const  ArrayIterator& other) const {verify_array(other); return index_ >= other.index_;}
        bool operator<=(const  ArrayIterator& other) const {verify_array(other); return index_ <= other.index_;}

    private:
        friend Array;
        ArrayIterator(size_t index, Array* array) : index_(static_cast<int64_t>(index)), array_(array) {}

        void verify_array(const ArrayIterator& other) const {
            if (array_ != other.array_) {
                throw std::invalid_argument("array iterators belong to different arrays");
            }
        }

        int64_t index_;
        Array* array_;
    };

    using value_type = T;
    using size_type = size_t;
    using difference_type = typename ArrayIterator<true>::difference_type;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = value_type*;
    using const_pointer = const value_type*;

    using iterator = ArrayIterator<false>;
    using const_iterator = ArrayIterator<true>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    // Creating
    Array() noexcept;

    // Exception guarantees destruction of created elements
    explicit Array(size_type size);
    Array(size_type size, const value_type& val);
    Array(std::initializer_list<value_type> list)
        requires concepts::move_constructible<T>;
    Array(std::initializer_list<value_type> list)
        requires concepts::only_copy_constructible<T>;

    Array(const Array &other);
    Array &operator=(const Array &other);

    Array(Array &&other) noexcept;
    Array &operator=(Array &&other) noexcept;

    ~Array();

    // Access
    // Nice one
    [[nodiscard]] reference at(size_type idx);
    [[nodiscard]] const_reference at(size_type idx) const;

    // UNSAFE
    [[nodiscard]] reference operator[](size_type idx);
    [[nodiscard]] const_reference operator[](size_type idx) const;

    // UNSAFE
    [[nodiscard]] reference front();
    [[nodiscard]] const_reference front() const;

    // UNSAFE
    [[nodiscard]] reference &back();
    [[nodiscard]] const_reference back() const;

    // Iterators
    iterator begin() noexcept;
    const_iterator begin() const noexcept;

    iterator end() noexcept;
    const_iterator end() const noexcept;

    reverse_iterator rbegin() noexcept;
    const_reverse_iterator rbegin() const noexcept;

    reverse_iterator rend() noexcept;
    const_reverse_iterator rend() const noexcept;

    // Capacity
    [[nodiscard]] bool empty()     const noexcept;
    [[nodiscard]] size_type size() const noexcept;
    void reserve(size_type new_cap)
        requires concepts::move_constructible<T>;
    void reserve(size_type new_cap)
        requires concepts::only_copy_constructible<T>;
    [[nodiscard]] size_type capacity() const noexcept;
    void shrink_to_fit();

    // Modifiers
    void clear();
    void push_back(const value_type& value);
    void push_back(value_type &&value);
    void pop_back();
    void swap(Array& other) noexcept;

    template<typename... Args>
    void emplace_back(Args &&... args) {
        check_expand();

        storage_.construct(size_++, std::forward<Args>(args)...);
    }

protected:
    Storage<T> storage_;
    size_type size_{};

private:
    void check_expand() requires concepts::move_constructible<T>;
    void check_expand() requires (concepts::copy_constructible<T> && !concepts::move_constructible<T>);

    [[nodiscard]] size_type calc_new_cap() const;

    void check_range(size_type idx) const;

    static constexpr size_type MIN_SIZE{4};
};

// ========================== Creating ========================================
// ----------------------------------------------------------------------------

template <typename T, template<typename StorageT> typename Storage>
Array<T, Storage>::Array() noexcept
    : size_(0) {
}

template <typename T, template<typename StorageT> typename Storage>
Array<T, Storage>::Array(size_type size)
    : Array(size, T()) {
}

template <typename T, template<typename StorageT> typename Storage>
Array<T, Storage>::Array(size_type size, const value_type& val)
    : Array() {
    if (size == 0) {
        return;
    }

    storage_.allocate(size);

    size_type idx = 0;
    try {
        for (; idx < size; ++idx) {
            storage_.construct(idx, val);
        }
    }
    catch (...) {
        while (idx--) {
            storage_.destruct(idx);
        }
        storage_.deallocate();
        throw;
    }

    size_ = size;
}

template <typename T, template<typename StorageT> typename Storage>
Array<T, Storage>::Array(std::initializer_list<value_type> list)
    requires concepts::move_constructible<T>
    : Array() {
    if (list.size() == 0) {
        return;
    }

    storage_.allocate(list.size());

    size_type idx = 0;
    try {
        auto it = list.begin();
        for (; idx < list.size(); ++idx, ++it) {
            storage_.construct(idx, std::move(*it));
        }
    }
    catch (...) {
        while (idx--) {
            storage_.destruct(idx);
        }
        storage_.deallocate();
        throw;
    }

    size_ = list.size();
}

template <typename T, template<typename StorageT> typename Storage>
Array<T, Storage>::Array(std::initializer_list<value_type> list)
    requires (concepts::copy_constructible<T> && !concepts::move_constructible<T>)
    : Array() {
    if (list.size() == 0) {
        return;
    }

    storage_.allocate(list.size());

    size_type idx = 0;
    try {
        auto it = list.begin();
        for (; idx < list.size(); ++idx, ++it) {
            storage_.construct(idx, *it);
        }
    }
    catch (...) {
        while (idx--) {
            storage_.destruct(idx);
        }
        storage_.deallocate();
        throw;
    }

    size_ = list.size();
}

template <typename T, template<typename StorageT> typename Storage>
Array<T, Storage>::Array(const Array& other)
    : Array() {
    if (other.empty()) {
        return;
    }

    storage_.allocate(other.size());

    size_type idx = 0;
    try {
        for (; idx < other.size(); ++idx) {
            storage_.construct(idx, other[idx]);
        }
    }
    catch (...) {
        while (idx--) {
            storage_.destruct(idx);
        }
        storage_.deallocate();
        throw;
    }

    size_ = other.size();
}

template <typename T, template<typename StorageT> typename Storage>
Array<T, Storage>& Array<T, Storage>::operator=(const Array& other) {
    if (this == &other) {
        return *this;
    }

    Array tmp(other);
    swap(tmp);

    return *this;
}

template <typename T, template<typename StorageT> typename Storage>
Array<T, Storage>::Array(Array&& other) noexcept {
    swap(other);
}

template <typename T, template<typename StorageT> typename Storage>
Array<T, Storage>& Array<T, Storage>::operator=(Array&& other) noexcept {
    if (this == &other) {
        return *this;
    }

    swap(other);
    return *this;
}

template <typename T, template<typename StorageT> typename Storage>
Array<T, Storage>::~Array() {
    clear();
    storage_.deallocate();
}

// ========================== Access =======================================
// ----------------------------------------------------------------------------

template <typename T, template<typename StorageT> typename Storage>
typename Array<T, Storage>::reference Array<T, Storage>::at(size_type idx) {
    check_range(idx);
    return storage_[idx];
}

template <typename T, template<typename StorageT> typename Storage>
typename Array<T, Storage>::const_reference Array<T, Storage>::at(size_type idx) const {
    check_range(idx);
    return storage_[idx];
}

template <typename T, template<typename StorageT> typename Storage>
const T& Array<T, Storage>::operator[](size_t idx) const {
    return storage_[idx];
}

template <typename T, template<typename StorageT> typename Storage>
T& Array<T, Storage>::operator[](size_t idx) {
    return storage_[idx];
}

template <typename T, template<typename StorageT> typename Storage>
T& Array<T, Storage>::front() {
    return storage_[0];
}

template <typename T, template<typename StorageT> typename Storage>
const T& Array<T, Storage>::front() const {
    return storage_[0];
}

template <typename T, template<typename StorageT> typename Storage>
T& Array<T, Storage>::back() {
    return storage_[size_ - 1];
}

template <typename T, template<typename StorageT> typename Storage>
const T& Array<T, Storage>::back() const {
    return storage_[size_ - 1];
}

// ========================== Iterators =======================================
// ----------------------------------------------------------------------------

template <typename T, template<typename StorageT> typename Storage>
typename Array<T, Storage>::iterator Array<T, Storage>::begin() noexcept {
    return iterator(0, this);
}

template <typename T, template<typename StorageT> typename Storage>
typename Array<T, Storage>::const_iterator Array<T, Storage>::begin() const noexcept {
    return const_iterator(0, this);
}

template <typename T, template<typename StorageT> typename Storage>
typename Array<T, Storage>::iterator Array<T, Storage>::end() noexcept {
    return iterator(size_, this);
}

template <typename T, template<typename StorageT> typename Storage>
typename Array<T, Storage>::const_iterator Array<T, Storage>::end() const noexcept {
    return const_iterator(size_, this);
}

template <typename T, template<typename StorageT> typename Storage>
typename Array<T, Storage>::reverse_iterator Array<T, Storage>::rbegin() noexcept {
    return reverse_iterator(size_, this);
}

template <typename T, template<typename StorageT> typename Storage>
typename Array<T, Storage>::const_reverse_iterator Array<T, Storage>::rbegin() const noexcept {
    return const_reverse_iterator(size_, this);
}

template <typename T, template<typename StorageT> typename Storage>
typename Array<T, Storage>::reverse_iterator Array<T, Storage>::rend() noexcept {
    return reverse_iterator(0, this);
}

template <typename T, template<typename StorageT> typename Storage>
typename Array<T, Storage>::const_reverse_iterator Array<T, Storage>::rend() const noexcept {
    return const_reverse_iterator(0, this);
}

// ========================== Capacity ========================================
// ----------------------------------------------------------------------------

template <typename T, template<typename StorageT> typename Storage>
bool Array<T, Storage>::empty() const noexcept {
    return size_ == 0;
}

template <typename T, template<typename StorageT> typename Storage>
size_t Array<T, Storage>::size() const noexcept {
    return size_;
}

template <typename T, template<typename StorageT> typename Storage>
void Array<T, Storage>::reserve(size_type new_cap) {

}

template <typename T, template<typename StorageT> typename Storage>
typename Array<T, Storage>::size_type Array<T, Storage>::capacity() const noexcept {
    return storage_.capacity();
}

template <typename T, template<typename StorageT> typename Storage>
void Array<T, Storage>::shrink_to_fit() {
    if (size() == capacity()) {
        return;
    }

    Array new_array(*this);
    swap(new_array);
}

// ========================== Modifiers =======================================
// ----------------------------------------------------------------------------

template <typename T, template<typename StorageT> typename Storage>
void Array<T, Storage>::clear() {
    while (!empty()) {
        pop_back();
    }
}

template <typename T, template<typename StorageT> typename Storage>
void Array<T, Storage>::push_back(const T& value) {
    emplace_back(value);
}

template <typename T, template<typename StorageT> typename Storage>
void Array<T, Storage>::push_back(T&& value) {
    emplace_back(std::move(value));
}

template <typename T, template<typename StorageT> typename Storage>
void Array<T, Storage>::pop_back() {
    size_--;
    storage_.destruct(size_);
}

template <typename T, template<typename StorageT> typename Storage>
void Array<T, Storage>::swap(Array& other) noexcept {
    storage_.swap(other.storage_);
    std::swap(size_, other.size_);
}

// ----------------------------------------------------------------------------

template <typename T, template<typename StorageT> typename Storage>
void Array<T, Storage>::check_expand()
    requires concepts::move_constructible<T> {
    if (size_ != storage_.capacity()) {
        return;
    }

    Array new_array;
    new_array.reserve(calc_new_cap());

    for (auto& el: *this) {
        new_array.emplace_back(std::move(el));
    }

    swap(new_array);
}

template <typename T, template<typename StorageT> typename Storage>
void Array<T, Storage>::check_expand()
    requires (concepts::copy_constructible<T> && !concepts::move_constructible<T>) {
    if (size_ != storage_.capacity()) {
        return;
    }

    Array new_array;
    new_array.reserve(calc_new_cap());

    for (auto& el: *this) {
        new_array.template emplace_back(el);
    }

    swap(new_array);
}

template <typename T, template<typename StorageT> typename Storage>
typename Array<T, Storage>::size_type Array<T, Storage>::calc_new_cap() const {
    return size_ == 0 ? MIN_SIZE : size_ * 2;
}

template <typename T, template<typename StorageT> typename Storage>
void Array<T, Storage>::check_range(size_type idx) const {
    if (idx >= storage_) {
        throw std::out_of_range("Array::check_range failed");
    }
}

} // nostd:;

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

// ============================================================================
