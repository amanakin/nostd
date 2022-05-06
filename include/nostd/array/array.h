#pragma once

#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <stdexcept>
#include <compare>

#include <nostd/concepts/concepts.h>
#include <nostd/storage/storage.h>

namespace nostd {

// ============================================================================

template <typename T, template<typename StorageT> typename Storage = storage::DynamicStorage>
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
        ArrayIterator operator++(int) noexcept { // NOLINT
            ArrayIterator prev(*this);
            this->operator++();
            return prev;
        }

        ArrayIterator& operator--() noexcept {--index_; return *this;}
        ArrayIterator operator--(int) noexcept { // NOLINT
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
    Array(std::initializer_list<value_type> list) requires move_constructible<T>;
    Array(std::initializer_list<value_type> list) requires only_copy_constructible<T>;

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
    void reserve(size_type new_cap);
    [[nodiscard]] size_type capacity() const noexcept;
    void shrink_to_fit();
    pointer data() noexcept;
    const_pointer data() const noexcept;

    // Modifiers
    void clear();
    void push_back(const value_type& value);
    void push_back(value_type &&value);
    void pop_back();
    void swap(Array& other) noexcept;

    template<typename... Args>
    void emplace_back(Args &&... args) {
        if (size() == capacity()) {
            emplace_back_resize(std::forward<Args>(args)...);
        } else {
            storage_.construct(size_++, std::forward<Args>(args)...);
        }
    }

protected:
    Storage<T> storage_;
    size_type size_{};

private:
    template<typename... Args>
    void emplace_back_resize(Args &&... args) requires move_constructible<T> {
        Array new_array;
        new_array.reserve(calc_new_cap());

        for (size_t idx = 0; idx < size(); ++idx) {
            new_array.emplace_back(std::move(operator[](idx)));
        }
        new_array.emplace_back( std::forward<Args>(args)...);

        swap(new_array);
    }
    template<typename... Args>
    void emplace_back_resize(Args &&... args) requires only_copy_constructible<T> {
        Array new_array;
        new_array.reserve(calc_new_cap());

        for (size_t idx = 0; idx < size(); ++idx) {
            new_array.emplace_back(operator[](idx));
        }
        new_array.emplace_back( std::forward<Args>(args)...);

        swap(new_array);
    }

    [[nodiscard]] size_type calc_new_cap() const;
    void check_range(size_type idx) const;
    void resize(size_type new_cap) requires move_constructible<T>;
    void resize(size_type new_cap) requires only_copy_constructible<T>;

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
Array<T, Storage>::Array(std::initializer_list<value_type> list) requires move_constructible<T>
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
Array<T, Storage>::Array(std::initializer_list<value_type> list) requires only_copy_constructible<T>
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
    return operator[](idx);
}

template <typename T, template<typename StorageT> typename Storage>
typename Array<T, Storage>::const_reference Array<T, Storage>::at(size_type idx) const {
    check_range(idx);
    return operator[](idx);
}

template <typename T, template<typename StorageT> typename Storage>
typename Array<T, Storage>::const_reference Array<T, Storage>::operator[](size_t idx) const {
    return storage_[idx];
}

template <typename T, template<typename StorageT> typename Storage>
typename Array<T, Storage>::reference Array<T, Storage>::operator[](size_t idx) {
    return storage_[idx];
}

template <typename T, template<typename StorageT> typename Storage>
typename Array<T, Storage>::reference Array<T, Storage>::front() {
    return operator[](0);
}

template <typename T, template<typename StorageT> typename Storage>
typename Array<T, Storage>::const_reference Array<T, Storage>::front() const {
    return operator[](0);
}

template <typename T, template<typename StorageT> typename Storage>
typename Array<T, Storage>::reference Array<T, Storage>::back() {
    return operator[](size_ - 1);
}

template <typename T, template<typename StorageT> typename Storage>
typename Array<T, Storage>::const_reference Array<T, Storage>::back() const {
    return operator[](size_ - 1);
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
    return size() == 0;
}

template <typename T, template<typename StorageT> typename Storage>
size_t Array<T, Storage>::size() const noexcept {
    return size_;
}

template <typename T, template<typename StorageT> typename Storage>
void Array<T, Storage>::reserve(size_type new_cap) {
    if (new_cap <= capacity()) {
        return;
    }

    resize(new_cap);
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

    resize(size());
}

template <typename T, template<typename StorageT> typename Storage>
typename Array<T, Storage>::pointer Array<T, Storage>::data() noexcept {
    return const_cast<pointer>(const_cast<const Array*>(this)->data());
}

template <typename T, template<typename StorageT> typename Storage>
typename Array<T, Storage>::const_pointer Array<T, Storage>::data() const noexcept {
    if (capacity() == 0) {
        return nullptr;
    }

    return &operator[](0);
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
    storage_.destruct(--size_);
}

template <typename T, template<typename StorageT> typename Storage>
void Array<T, Storage>::swap(Array& other) noexcept {
    storage_.swap(other.storage_);
    std::swap(size_, other.size_);
}

// ----------------------------------------------------------------------------

template <typename T, template<typename StorageT> typename Storage>
typename Array<T, Storage>::size_type Array<T, Storage>::calc_new_cap() const {
    return size_ == 0 ? MIN_SIZE : size_ * 2;
}

template <typename T, template<typename StorageT> typename Storage>
void Array<T, Storage>::check_range(size_type idx) const {
    if (idx >= size()) {
        throw std::out_of_range("Array::check_range failed");
    }
}

template <typename T, template<typename StorageT> typename Storage>
void Array<T, Storage>::resize(size_type new_cap) requires move_constructible<T> {
    if (new_cap < size()) {
        return;
    }

    Array new_array;
    new_array.storage_.allocate(new_cap);

    for (size_type idx = 0; idx < size(); ++idx) {
        new_array.emplace_back(std::move(operator[](idx)));
    }

    swap(new_array);
}

template <typename T, template<typename StorageT> typename Storage>
void Array<T, Storage>::resize(size_type new_cap) requires only_copy_constructible<T> {
    if (new_cap < size()) {
        return;
    }

    Array new_array;
    new_array.storage_.allocate(new_cap);

    for (size_type idx = 0; idx < size(); ++idx) {
        new_array.emplace_back(operator[](idx));
    }

    swap(new_array);
}

// ============================================================================
// ============================================================================
// ============================================================================

template <template <typename StorageType> typename Storage>
struct Array<bool, Storage> {
private:
    struct Reference {
        Reference(uint8_t* chunk, uint8_t offset)
            : chunk_(chunk), offset_(offset) {
        }

        operator bool() const { // NOLINT
            return (*chunk_) & (1 << offset_);
        }

        Reference(const Reference& other) {
            chunk_ = other.chunk_;
            offset_ = other.offset_;
        }

        Reference(Reference&& other) noexcept {
            chunk_ = other.chunk_;
            offset_ = other.offset_;
        }

        Reference& operator=(const Reference& other) {
            if (this == &other) {
                return *this;
            }
            return operator=(static_cast<bool>(other)); // NOLINT
        }

        Reference& operator=(Reference&& other) noexcept {
            return operator=(other); // NOLINT
        }

        friend std::strong_ordering operator<=>(const Reference& lhs, const Reference& rhs) {
            if (auto cmp = lhs.chunk_ <=> rhs.chunk_; cmp != 0) {
                return cmp;
            }
            return lhs.offset_ <=> rhs.offset_;
        }

        Reference& operator=(bool other) {
            if (other) {
                *chunk_ = (*chunk_) | (1 << offset_);
            } else {
                *chunk_ = (*chunk_) & (~(1 << offset_));
            }

            return *this;
        }

        uint8_t* chunk_;
        uint8_t offset_;
    };

public:
    template <bool isConst>
    class ArrayIterator {
    public:
        using difference_type = int64_t;
        using iterator_category = std::random_access_iterator_tag;

        using value_type = std::conditional_t<isConst, const bool, bool>;
        using reference  = std::conditional_t<isConst, const Reference, Reference>;

        bool operator==(const ArrayIterator& other) const {
            return ref_.chunk_ == other.ref_.chunk_ && ref_.offset_ == other.ref_.offset_;
        }
        bool operator!=(const ArrayIterator& other) const {
            return ref_.chunk_ != other.ref_.chunk_ || ref_.offset_ != other.ref_.offset_;
        }

        reference operator*() const {return ref_;}

        ArrayIterator& operator++() noexcept {
            if (ref_.offset_ != 7) {
                ref_.offset_++;
            } else {
                ref_.offset_ = 0;
                ref_.chunk_++;
            }

            return *this;
        }
        ArrayIterator operator++(int) noexcept { // NOLINT
            ArrayIterator prev(*this);
            this->operator++();
            return prev;
        }

        ArrayIterator& operator--() noexcept {
            if (ref_.offset_ != 0) {
                ref_.offset_--;
            } else {
                ref_.offset_ = 7;
                ref_.chunk_--;
            }

            return *this;
        }
        ArrayIterator operator--(int) noexcept {
            ArrayIterator prev(*this);
            this->operator--();
            return prev;
        }

        ArrayIterator& operator+=(difference_type diff) noexcept {
            ref_.offset_ += diff;

            ref_.chunk_ += (ref_.offset_ / 8);
            ref_.offset_ %= 8;

            return *this;
        }
        ArrayIterator& operator-=(difference_type diff) noexcept {
            auto offset_diff = diff % 8;
            auto chunk_diff =  diff / 8;

            if (ref_.offset_ < offset_diff) {
                ref_.offset_ -= offset_diff - 8;
                ref_.chunk_  -= chunk_diff + 1;
            } else {
                ref_.offset_ -= offset_diff;
                ref_.chunk_  -=   chunk_diff;
            }


            return *this;
        }

        ArrayIterator operator+(difference_type diff) const noexcept {
            ArrayIterator it(*this);
            it += diff;
            return it;
        }
        ArrayIterator operator-(difference_type diff) const noexcept {
            ArrayIterator it(*this);
            it -= diff;
            return it;
        }

        difference_type operator-(ArrayIterator& other) const {
            difference_type chunk_diff = ref_.chunk_ - other.ref_.chunk_;
            if (ref_.offset_ < other.ref_.offset_) {
                return ((ref_.offset_ + 8) - other.ref_.offset_) + 8 * (chunk_diff - 1);
            }
            return (ref_.offset_ - other.ref_.offset_) + 8 * (chunk_diff);
        }

        friend std::strong_ordering operator<=>(const ArrayIterator& lhs, const ArrayIterator& rhs) {
            return lhs.ref_ <=> rhs.ref_;
        }

    private:
        friend Array;
        ArrayIterator(size_t index, Array* array) : ref_(array->operator[](index)) {}

        Reference ref_;
    };

    using value_type = bool;
    using size_type = size_t;
    using difference_type = typename ArrayIterator<true>::difference_type;
    using reference = Reference;
    using const_reference = bool;

    using iterator = ArrayIterator<false>;
    using const_iterator = ArrayIterator<true>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    // Creating
    Array() noexcept = default;

    // Exception guarantees destruction of created elements
    explicit Array(size_type size);
    Array(size_type size, value_type val);
    Array(std::initializer_list<value_type> list);

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
    [[nodiscard]] reference back();
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
    void reserve(size_type new_cap);
    [[nodiscard]] size_type capacity() const noexcept;
    void shrink_to_fit();

    // Modifiers
    void clear();
    void push_back(value_type value);
    void pop_back();
    void swap(Array& other) noexcept;

    void emplace_back(value_type value) {
        if (size() == capacity()) {
            emplace_back_resize(value);
        } else {
            operator[](size_++) = value;
        }
    }

protected:
    Storage<uint8_t> storage_;
    size_type size_{}; // count of bits

private:
    void emplace_back_resize(value_type value);

    [[nodiscard]] size_type calc_new_cap() const;
    [[nodiscard]] size_type real_size() const;

    void check_range(size_type idx) const;
    void resize(size_type new_cap);

    static constexpr size_type MIN_SIZE{4};
};

// ========================== Creating ========================================
// ----------------------------------------------------------------------------

template <template<typename StorageT> typename Storage>
Array<bool, Storage>::Array(size_type size)
    : Array(size, false) {
}

template <template<typename StorageT> typename Storage>
Array<bool, Storage>::Array(size_type size, value_type val)
    : size_(size) {
    if (size == 0) {
        return;
    }

    storage_.allocate(real_size());

    size_type idx = 0;
    for (; idx < size; ++idx) {
        operator[](idx) = val;
    }
}

template <template<typename StorageT> typename Storage>
Array<bool, Storage>::Array(std::initializer_list<value_type> list)
    : size_(list.size()) {
    if (list.size() == 0) {
        return;
    }

    storage_.allocate(real_size());

    size_type idx = 0;
    auto it = list.begin();
    for (; idx < list.size(); ++idx, ++it) {
        operator[](idx) = *it;
    }
}

template <template <typename StorageT> typename Storage>
Array<bool, Storage>::Array(const Array& other)
    : size_(other.size()) {
    if (other.empty()) {
        return;
    }

    storage_.allocate(real_size());

    size_type idx = 0;
    for (; idx < other.size(); ++idx) {
        storage_.construct(idx, other[idx]);
    }

    size_ = other.size();
}

template <template <typename StorageT> typename Storage>
Array<bool, Storage>& Array<bool, Storage>::operator=(const Array& other) {
    if (this == &other) {
        return *this;
    }

    Array tmp(other);
    swap(tmp);

    return *this;
}

template <template<typename StorageT> typename Storage>
Array<bool, Storage>::Array(Array&& other) noexcept {
    swap(other);
}

template <template<typename StorageT> typename Storage>
Array<bool, Storage>& Array<bool, Storage>::operator=(Array&& other) noexcept {
    if (this == &other) {
        return *this;
    }

    swap(other);
    return *this;
}

template <template<typename StorageT> typename Storage>
Array<bool, Storage>::~Array() {
    clear();
    storage_.deallocate();
}

// ========================== Access =======================================
// ----------------------------------------------------------------------------

template <template <typename StorageT> typename Storage>
typename Array<bool, Storage>::reference
Array<bool, Storage>::at(size_type idx) {
    check_range(idx);
    return operator[](idx);
}

template <template<typename StorageT> typename Storage>
typename Array<bool, Storage>::const_reference
Array<bool, Storage>::at(size_type idx) const {
    check_range(idx);
    return operator[](idx);
}

template <template <typename StorageType> typename Storage>
typename Array<bool, Storage>::const_reference
Array<bool, Storage>::operator[](size_t idx) const {
    return storage_[idx >> 3] & (1 << (idx & 7));
}

template <template <typename StorageType> typename Storage>
typename Array<bool, Storage>::reference
Array<bool, Storage>::operator[](size_t idx) {
    return Reference(&storage_[idx >> 3], (idx & 7));
}

template <template <typename StorageT> typename Storage>
typename Array<bool, Storage>::reference Array<bool, Storage>::front() {
    return operator[](0);
}

template <template <typename StorageT> typename Storage>
typename Array<bool, Storage>::const_reference Array<bool, Storage>::front() const {
    return operator[](0);
}

template <template <typename StorageT> typename Storage>
typename Array<bool, Storage>::reference Array<bool, Storage>::back() {
    return operator[](size() - 1);
}

template <template <typename StorageT> typename Storage>
typename Array<bool, Storage>::const_reference Array<bool, Storage>::back() const {
    return operator[](size() - 1);
}

// ========================== Iterators =======================================
// ----------------------------------------------------------------------------


template <template <typename StorageT> typename Storage>
typename Array<bool, Storage>::iterator Array<bool, Storage>::begin() noexcept {
    return iterator(0, this);
}

template <template <typename StorageT> typename Storage>
typename Array<bool, Storage>::const_iterator Array<bool, Storage>::begin() const noexcept {
    return const_iterator(0, this);
}

template <template <typename StorageT> typename Storage>
typename Array<bool, Storage>::iterator Array<bool, Storage>::end() noexcept {
    return iterator(size_, this);
}

template <template <typename StorageT> typename Storage>
typename Array<bool, Storage>::const_iterator Array<bool, Storage>::end() const noexcept {
    return const_iterator(size_, this);
}

template <template<typename StorageT> typename Storage>
typename Array<bool, Storage>::reverse_iterator Array<bool, Storage>::rbegin() noexcept {
    return reverse_iterator(size_, this);
}

template <template<typename StorageT> typename Storage>
typename Array<bool, Storage>::const_reverse_iterator Array<bool, Storage>::rbegin() const noexcept {
    return const_reverse_iterator(size_, this);
}

template <template<typename StorageT> typename Storage>
typename Array<bool, Storage>::reverse_iterator Array<bool, Storage>::rend() noexcept {
    return reverse_iterator(0, this);
}

template <template<typename StorageT> typename Storage>
typename Array<bool, Storage>::const_reverse_iterator Array<bool, Storage>::rend() const noexcept {
    return const_reverse_iterator(0, this);
}

// ========================== Capacity ========================================
// ----------------------------------------------------------------------------

template <template<typename StorageT> typename Storage>
bool Array<bool, Storage>::empty() const noexcept {
    return size() == 0;
}

template <template<typename StorageT> typename Storage>
size_t Array<bool, Storage>::size() const noexcept {
    return size_;
}

template <template<typename StorageT> typename Storage>
void Array<bool, Storage>::reserve(size_type new_cap) {
    if (new_cap <= capacity()) {
        return;
    }

    resize(new_cap);
}

template <template<typename StorageT> typename Storage>
typename Array<bool, Storage>::size_type Array<bool, Storage>::capacity() const noexcept {
    return storage_.capacity() * 8;
}

template <template<typename StorageT> typename Storage>
void Array<bool, Storage>::shrink_to_fit() {
    if (size() == capacity()) {
        return;
    }

    resize(size());
}

// ========================== Modifiers =======================================
// ----------------------------------------------------------------------------

template <template<typename StorageT> typename Storage>
void Array<bool, Storage>::clear() {
    while (!empty()) {
        pop_back();
    }
}

template <template<typename StorageT> typename Storage>
void Array<bool, Storage>::push_back(value_type value) {
    emplace_back(value);
}

template <template<typename StorageT> typename Storage>
void Array<bool, Storage>::pop_back() {
    --size_;
}

template <template<typename StorageT> typename Storage>
void Array<bool, Storage>::swap(Array& other) noexcept {
    storage_.swap(other.storage_);
    std::swap(size_, other.size_);
}

// ----------------------------------------------------------------------------

template <template<typename StorageT> typename Storage>
typename Array<bool, Storage>::size_type Array<bool, Storage>::calc_new_cap() const {
    return size() == 0 ? MIN_SIZE : size() * 2;
}

template <template<typename StorageT> typename Storage>
void Array<bool, Storage>::check_range(size_type idx) const {
    if (idx >= size()) {
        throw std::out_of_range("Array::check_range failed");
    }
}

template <template <typename StorageT> typename Storage>
typename Array<bool, Storage>::size_type Array<bool, Storage>::real_size() const {
    return size() / 8 + (size() % 8 == 0 ? 0 : 1);
}

template <template <typename StorageT> typename Storage>
void Array<bool, Storage>::emplace_back_resize(value_type value) {
    Array new_array;


    new_array.reserve(calc_new_cap());

    for (size_t idx = 0; idx < size(); ++idx) {
        new_array.push_back(operator[](idx));
    }
    new_array.emplace_back(value);

    swap(new_array);
}

template <template<typename StorageT> typename Storage>
void Array<bool, Storage>::resize(size_type new_cap) {
    if (new_cap < size()) {
        return;
    }

    Array new_array;
    new_array.storage_.allocate(new_cap / 8 + 1);

    for (size_type idx = 0; idx < size(); ++idx) {
        new_array[idx] = operator[](idx);
    }

    swap(new_array);
}

} // nostd

// ============================================================================
