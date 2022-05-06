#pragma once

#include "gtest/gtest.h"
#include "unordered_set"

template<typename T>
T const &as_const(T &obj) {
    return obj;
}

template<typename T>
struct Tricky {
    Tricky() {
        add_instance();
    }

    Tricky(T const &val) : val(val) { // NOLINT
        add_instance();
    }

    Tricky(Tricky const &other) : val(other.val) {
        copy();
        add_instance();
    }

    Tricky &operator=(Tricky const &other) {
        if (this == &other) {
            return *this;
        }

        assert_exists();
        other.assert_exists();
        copy();
        val = other.val;
        return *this;
    }

    ~Tricky() {
        delete_instance();
    }

    static std::unordered_set<Tricky const *> &instances() {
        static std::unordered_set<Tricky const *> instances;
        return instances;
    }

    static void expect_no_instances() {
        if (!instances().empty()) {
            FAIL() << "not all instances are destroyed";
        }
    }

    T& get() {
        return val;
    }

    friend bool operator==(Tricky const &a, Tricky const &b) {
        return a.val == b.val;
    }

    friend bool operator!=(Tricky const &a, Tricky const &b) {
        return a.val != b.val;
    }

private:
    void add_instance() {
        auto p = instances().insert(this);
        if (!p.second) {
            FAIL() << "a new object is created at the address "
                   << static_cast<void *>(this)
                   << " while the previous object at this address was not destroyed";
        }
    }

    void delete_instance() {
        size_t erased = instances().erase(this);
        if (erased != 1) {
            FAIL() << "attempt of destroying non-existing object at address "
                   << static_cast<void *>(this);
        }
    }

    void assert_exists() const {
        std::unordered_set<Tricky const *> const &inst = instances();
        bool exists = inst.find(this) != inst.end();
        if (!exists) {
            FAIL() << "accessing an non-existsing object at address "
                   << static_cast<void const *>(this);
        }
    }

    void copy() {
    }

    T val;
};

