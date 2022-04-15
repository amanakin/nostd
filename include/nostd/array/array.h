#pragma once

#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <stdexcept>

#include <nostd/util.h>
#include <nostd/storage/storage.h>

namespace nostd {

// ============================================================================

template<
        typename T,
        template<typename StorageType> typename Storage
        >
struct Array : public Storage<T> {

    template <bool isConst>
    class ArrayIterator {
    public:
        using difference_type = int64_t;
        using iterator_category = std::random_access_iterator_tag;

        using value_type = std::conditional_t<isConst, const T, T>;
        using pointer   = std::conditional_t<isConst, const T *, T *>;
        using reference = std::conditional_t<isConst, const T &, T &>;

        ArrayIterator(size_t index, Array* array) : index_(index), array_(array) {}

        bool operator==(const ArrayIterator& other) const {return index_ == other.index_ && array_ == other.array_;}
        bool operator!=(const ArrayIterator& other) const {return index_ != other.index_ || array_ != other.array_;}

        reference operator*() const {return (*array_)[index_];}
        pointer operator->() const {return &((*array_)[index_]);}

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

        bool operator>(const  ArrayIterator& other) const {verify_array(other); return index_ > other.index_;}
        bool operator<(const  ArrayIterator& other) const {verify_array(other); return index_ < other.index_;}
        bool operator>=(const ArrayIterator& other) const {verify_array(other); return index_ >= other.index_;}
        bool operator<=(const ArrayIterator& other) const {verify_array(other); return index_ <= other.index_;}

    private:
        void verify_array(const ArrayIterator& other) const {
            if (array_ != other.array_) {
                throw std::invalid_argument("array iterators have different array pointers in operator-");
            }
        }

        size_t index_;
        Array* array_;
    };

    using iterator = ArrayIterator<false>;
    using const_iterator = ArrayIterator<true>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    Array() noexcept;
    explicit Array(size_t size);
    Array(size_t size, const T &val);
    Array(std::initializer_list<T> list);

    Array(const Array &other);
    Array &operator=(const Array &other);

    Array(Array &&other) noexcept;
    Array &operator=(Array &&other) noexcept;

    ~Array() = default;

    // Access
    [[nodiscard]] const T &operator[](size_t idx) const;
    [[nodiscard]] T &operator[](size_t idx);

    [[nodiscard]] T &front();
    [[nodiscard]] const T &front() const;

    [[nodiscard]] T &back();
    [[nodiscard]] const T &back() const;

    // Storage
    [[nodiscard]] bool empty() const noexcept;
    [[nodiscard]] size_t size() const noexcept;
    void resize(size_t size);
    void clear() noexcept;

    // Modifiers
    template<typename... Args>
    void emplace_back(Args &&... args) {
        Storage<T>::emplace_back(std::forward<Args>(args)...);
    }

    void push_back(const T &value);
    void push_back(T &&value);

    void pop_back();

    // Iterators
    iterator begin() noexcept;
    const_iterator cbegin() const noexcept;
    const_iterator begin() const noexcept;

    iterator end() noexcept;
    const_iterator cend() const noexcept;
    const_iterator end() const noexcept;

protected:
    void expand_capacity();

    size_t size_{0};

    using Storage<T>::data;
    using Storage<T>::capacity;

private:
};

// ----------------------------------------------------------------------------

template<
        typename T,
        template<typename StorageType> typename Storage
        >
Array<T, Storage>::Array() noexcept
    : size_(0) {
}

template<
        typename T,
        template<typename StorageType> typename Storage
        >
Array<T, Storage>::Array(size_t size)
        : size_(size), Storage<T>(size) {
}

template<
        typename T,
        template<typename StorageType> typename Storage
>
Array<T, Storage>::Array(size_t size, const T &val)
        : size_(size), Storage<T>(size) {
    for (size_t idx = 0; idx < size; ++idx) {
        data(idx) = val;
    }
}


template<
        typename T,
        template<typename StorageType> typename Storage
>
Array<T, Storage>::Array(std::initializer_list<T> list)
        : size_(list.size()), Storage<T>(list.size()) {
    size_t idx = 0;
    for (auto &val : list) {
        data(idx++) = std::move(val);
    }
}

template<
        typename T,
        template<typename StorageType> typename Storage
        >
Array<T, Storage>::Array(const Array& other)
    : Storage<T>(other) {
    if (this == &other) {
        return;
    }

    size_ = other.size_;
}

template<
        typename T,
        template<typename StorageType> typename Storage
        >
Array<T, Storage>& Array<T, Storage>::operator=(const Array& other) {
    if (this == &other) {
        return *this;
    }

    static_cast<Storage<T>&>(*this) = other;
    size_ = other.size_;
}


template<
        typename T,
        template<typename StorageType> typename Storage
        >
Array<T, Storage>::Array(Array&& other) noexcept
    : Storage<T>(other) {
    if (this == &other) {
        return;
    }

    size_ = other.size_;
}

template<
        typename T,
        template<typename StorageType> typename Storage
        >
Array<T, Storage>& Array<T, Storage>::operator=(Array&& other) noexcept {
    if (this == &other) {
        return *this;
    }

    static_cast<Storage<T>&>(*this) = other;
    size_ = other.size_;
    return *this;
}

template <
        typename T,
        template <typename StorageType> typename Storage
        >
const T& Array<T, Storage>::operator[](size_t idx) const {
    return data(idx);
}

template <
        typename T,
        template <typename StorageType> typename Storage
        >
T& Array<T, Storage>::operator[](size_t idx) {
    return data(idx);
}

template <
        typename T,
        template <typename StorageType> typename Storage
        >
T& Array<T, Storage>::front() {
    return data(0);
}

template <
        typename T,
        template <typename StorageType> typename Storage
        >
const T& Array<T, Storage>::front() const {
    return data(0);
}

template <
        typename T,
        template <typename StorageType> typename Storage
        >
T& Array<T, Storage>::back() {
    return data(size_ - 1);
}

template <
        typename T,
        template <typename StorageType> typename Storage
        >
const T& Array<T, Storage>::back() const {
    return data(size_ - 1);
}

template <
        typename T,
        template <typename StorageType> typename Storage
        >
bool Array<T, Storage>::empty() const noexcept {
    return size_ == 0;
}

template <
        typename T,
        template <typename StorageType> typename Storage
        >
size_t Array<T, Storage>::size() const noexcept {
    return size_;
}

template <
        typename T,
        template <typename StorageType> typename Storage
        >
void Array<T, Storage>::resize(size_t size) {
    Storage<T>::resize(size);
}

template <
        typename T,
        template <typename StorageType> typename Storage
        >
void Array<T, Storage>::clear() noexcept {
    Storage<T>::resize(0);
}

template <
        typename T,
        template <typename StorageType> typename Storage
        >
void Array<T, Storage>::push_back(const T& value) {
    if (size_ == capacity()) {
        expand_capacity();
    }

    data(size_++) = std::move(value);
}

template <
        typename T,
        template <typename StorageType> typename Storage
        >
void Array<T, Storage>::push_back(T&& value) {
    if (size_ == capacity()) {
        expand_capacity();
    }

    data(size_++) = std::move(value);
}

template <
        typename T,
        template <typename StorageType> typename Storage
        >
void Array<T, Storage>::pop_back() {
    if (size_ == 0) {
        return;
    }

    size_--;
}

template <
        typename T,
        template <typename StorageType> typename Storage
        >
void Array<T, Storage>::expand_capacity() {
    Storage<T>::resize(size_ * 2);
}

template <
        typename T,
        template <typename StorageType> typename Storage
>
typename Array<T, Storage>::iterator Array<T, Storage>::begin() noexcept {
    return iterator(0, this);
}

template <
        typename T,
        template <typename StorageType> typename Storage
>
typename Array<T, Storage>::const_iterator Array<T, Storage>::cbegin() const noexcept {
    return const_iterator(0, this);
}

template <
        typename T,
        template <typename StorageType> typename Storage
        >
typename Array<T, Storage>::const_iterator Array<T, Storage>::begin() const noexcept {
    return cbegin();
}

template <
        typename T,
        template <typename StorageType> typename Storage
>
typename Array<T, Storage>::iterator Array<T, Storage>::end() noexcept {
    return iterator(size_, this);
}

template <
        typename T,
        template <typename StorageType> typename Storage
>
typename Array<T, Storage>::const_iterator Array<T, Storage>::cend() const noexcept {
    return const_iterator (size_, this);
}

template <
        typename T,
        template <typename StorageType> typename Storage
>
typename Array<T, Storage>::const_iterator Array<T, Storage>::end() const noexcept {
    return cend();
}

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


} // nostd

// ============================================================================
