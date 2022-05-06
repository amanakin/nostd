#pragma once

#include <compare>

namespace nostd::detail {

// ----------------------------------------------------------------------------

struct SharedCounter {
    inline explicit SharedCounter(size_t shared_cnt) noexcept;
    virtual ~SharedCounter() = default;

    virtual void on_zero_shared() noexcept = 0;

    inline void add_shared()     noexcept;
    inline void release_shared() noexcept;
    inline size_t use_count() const noexcept;

protected:
    std::atomic<size_t> shared_cnt_{0};
};

SharedCounter::SharedCounter(size_t shared_cnt) noexcept
    : shared_cnt_(shared_cnt) {
}

void SharedCounter::add_shared() noexcept {
    shared_cnt_.fetch_add(1, std::memory_order_relaxed);
}

void SharedCounter::release_shared() noexcept {
    if (shared_cnt_.fetch_sub(1, std::memory_order_acq_rel) == 1) {
        on_zero_shared();
    }
}

size_t SharedCounter::use_count() const noexcept {
    return shared_cnt_.load(std::memory_order_relaxed);
}

// ----------------------------------------------------------------------------

template <typename T>
struct CtrlPointer : public SharedCounter {
    explicit CtrlPointer(T* ptr) noexcept;

    void  on_zero_shared() noexcept override;

    using SharedCounter::add_shared;
    using SharedCounter::release_shared;

protected:
    T* obj_ptr_{nullptr};
};

template <typename T>
CtrlPointer<T>::CtrlPointer(T *ptr) noexcept
    : SharedCounter(1), obj_ptr_(ptr) {
}

template <typename T>
void CtrlPointer<T>::on_zero_shared() noexcept {
    delete obj_ptr_;
    delete this;
}

// ----------------------------------------------------------------------------

template <typename T>
struct CtrlInPlace : public SharedCounter {
    template <typename... Args>
    explicit CtrlInPlace(Args... args)
        : SharedCounter(1) {
        new (&obj_) T(std::forward<Args>(args)...);
    }

    void on_zero_shared() noexcept override;
    T* get() noexcept;

    using SharedCounter::add_shared;
    using SharedCounter::release_shared;

protected:
    std::aligned_storage<sizeof(T), alignof(T)> obj_;
};

template <typename T>
void CtrlInPlace<T>::on_zero_shared() noexcept {
    get()->~T();
    delete this;
}

template <typename T>
T* CtrlInPlace<T>::get() noexcept {
    return reinterpret_cast<T*>(&obj_);
}

} // nostd::detail

// ----------------------------------------------------------------------------

namespace nostd {

template <typename T>
class SharedPtr {
    using size_type  = std::size_t;
    using value_type = T;

public:
    SharedPtr() noexcept = default;
    explicit SharedPtr(std::nullptr_t) noexcept;

    /*
     * template constructors provide an option
     * to not qualify types if U = T[], etc.
     */
    template <typename U> explicit SharedPtr(U* ptr);
    SharedPtr(const SharedPtr& other) noexcept;
    template <typename U> explicit SharedPtr(const SharedPtr<U>& other) noexcept;
    SharedPtr(SharedPtr&& other) noexcept;
    template <typename U> explicit SharedPtr(SharedPtr<U>&& other) noexcept;

    // -----------------------------------------------------------------

    SharedPtr& operator=(const SharedPtr& other) noexcept;
    template <typename U> SharedPtr& operator=(const SharedPtr<U>& other) noexcept;
    SharedPtr& operator=(SharedPtr&&) noexcept;
    template <typename U> SharedPtr& operator=(SharedPtr<U>&& other) noexcept;

    // -----------------------------------------------------------------

    ~SharedPtr();

    // -----------------------------------------------------------------

    void reset() noexcept;
    template <typename U> void reset(U* ptr);

    void swap(SharedPtr& other) noexcept;
    value_type* get() const noexcept;

    value_type& operator *() const noexcept;
    value_type* operator->() const noexcept;

    [[nodiscard]] size_type use_count() const noexcept;

    explicit operator bool() const noexcept;

    template <typename U, typename... Args>
    friend SharedPtr<U> MakeShared(Args&&... args);

    template <typename U>
    friend class SharedPtr;

protected:
    detail::SharedCounter* block_{nullptr};
    T* ptr_{nullptr};
};

// ----------------------------------------------------------------------------

template <typename T>
SharedPtr<T>::SharedPtr(std::nullptr_t) noexcept {
}

template <typename T>
template <typename U>
SharedPtr<T>::SharedPtr(U* ptr)
    : ptr_(ptr) {
    block_ = new detail::CtrlPointer<U>(ptr);
}

template <typename T>
SharedPtr<T>::SharedPtr(const SharedPtr& other) noexcept {
    if (other) {
        ptr_ = other.ptr_;
        block_ = other.block_;
        block_->add_shared();
    }
}

template <typename T>
template <typename U>
SharedPtr<T>::SharedPtr(const SharedPtr<U>& other) noexcept  {
    if (other) {
        ptr_ = other.ptr_;
        block_ = other.block_;
        block_->add_shared();
    }
}

template <typename T>
SharedPtr<T>::SharedPtr(SharedPtr&& other) noexcept {
    swap(other);
}

template <typename T>
template <typename U>
SharedPtr<T>::SharedPtr(SharedPtr<U>&& other) noexcept {
    swap(other);
}

template <typename T>
SharedPtr<T>& SharedPtr<T>::operator=(const SharedPtr& other) noexcept {
    SharedPtr(other).swap(*this);
    return *this;
}

template <typename T>
template <typename U>
SharedPtr<T>& SharedPtr<T>::operator=(const SharedPtr<U>& other) noexcept {
    SharedPtr<T>(other).swap(*this);
    return *this;
}

template <typename T>
SharedPtr<T>& SharedPtr<T>::operator=(SharedPtr&& other) noexcept {
    SharedPtr(std::move(other)).swap(*this);
    return *this;
}

template <typename T>
template <typename U>
SharedPtr<T>& SharedPtr<T>::operator=(SharedPtr<U>&& other) noexcept {
    SharedPtr<T>(std::move(other)).swap(*this);
    return *this;
}

template <typename T>
SharedPtr<T>::~SharedPtr() {
    if (block_ != nullptr) {
        block_->release_shared();
    }
}

template <typename T>
void SharedPtr<T>::reset() noexcept {
    SharedPtr().swap(*this);
}

template <typename T>
template <typename U>
void SharedPtr<T>::reset(U* ptr) {
    SharedPtr<T>(ptr).swap(*this);
}

template <typename T>
void SharedPtr<T>::swap(SharedPtr& other) noexcept {
    std::swap(block_, other.block_);
    std::swap(ptr_, other.ptr_);
}

template <typename T>
typename SharedPtr<T>::value_type* SharedPtr<T>::get() const noexcept {
    return ptr_;
}

template <typename T>
typename SharedPtr<T>::value_type& SharedPtr<T>::operator *() const noexcept {
    return *get();
}

template <typename T>
typename SharedPtr<T>::value_type* SharedPtr<T>::operator->() const noexcept {
    return get();
}

template <typename T>
typename SharedPtr<T>::size_type SharedPtr<T>::use_count() const noexcept {
    if (block_ == nullptr) {
        return 0;
    }
    return block_->use_count();
}

template <typename T>
SharedPtr<T>::operator bool() const noexcept {
    return get() != nullptr;
}

template <typename T, typename... Args>
SharedPtr<T> MakeShared(Args&&... args) {
    SharedPtr<T> shared;
    auto block = new detail::CtrlInPlace<T>(std::forward<Args>(args)...);
    shared.block_ = block;
    shared.ptr_   = block->get();

    return shared;
}

template <typename T, typename U>
bool operator==(const SharedPtr<T>& lhs,
                const SharedPtr<U>& rhs) noexcept {
    return lhs.get() == rhs.get();
}

template <typename T>
bool operator==(const SharedPtr<T>& lhs,
                std::nullptr_t) noexcept {
    return lhs.get() == nullptr;
}

template <typename T, typename U>
std::strong_ordering operator<=>(const SharedPtr<T>& lhs,
                                 const SharedPtr<U>& rhs ) noexcept {
    return lhs.get() <=> rhs.get();
}

template <typename T>
std::strong_ordering operator<=>(const SharedPtr<T>& lhs,
                                 std::nullptr_t ) noexcept {
    return lhs.get() <=> nullptr;
}

// ----------------------------------------------------------------------------

} // nostd