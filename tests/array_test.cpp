#include <gtest/gtest.h>

#include <nostd/array/array.h>

#include "test_util.h"

#include <algorithm>

template <typename Array>
void TestAccess(Array& array) {
    for (size_t idx = 0; idx < array.size(); ++idx) {
        array[idx] = static_cast<int>(idx * 2);
    }

    for (size_t idx = 0; idx < array.size(); ++idx) {
        ASSERT_EQ(array[idx], idx * 2);
    }
};

template <typename Array>
void TestEqual(const Array& lhs, const Array& rhs) {
    ASSERT_EQ(lhs.size(), rhs.size());
    for (size_t idx = 0; idx < lhs.size(); ++idx) {
        ASSERT_EQ(lhs[idx], rhs[idx]);
    }
}

template <typename Storage>
void TestMove(const Storage& storage) {
    auto copy(storage);

    auto moved(std::move(copy));
    TestEqual(moved, storage);
}

template <typename Storage>
void TestAssign(Storage& lhs, const Storage& rhs) {
    auto copy(rhs);

    lhs = rhs;
}

TEST(ConstructTest, DefaultConstruct) {
    nostd::Array<Tricky<int> > a;
    Tricky<int>::expect_no_instances();
    EXPECT_TRUE(a.empty());
    EXPECT_EQ(0, a.size());
}

TEST(ConstructTest, SizeConstruct) {
    nostd::Array<int> a(10);
    ASSERT_EQ(a.size(), 10);

    TestAccess(a);
}

TEST(ConstructTest, InitList) {
    nostd::Array<int> a({0xEDA, 0xEDA, 0xEDA});
    ASSERT_EQ(a.size(), 3);
    for (size_t idx = 0; idx < a.size(); ++idx) {
        ASSERT_EQ(a[idx], 0xEDA);
    }
}

TEST(ConstructTest, SizeVal) {
    nostd::Array<int> a(10, 0xEDA);
    ASSERT_EQ(a.size(), 10);
    for (size_t idx = 0; idx < a.size(); ++idx) {
        ASSERT_EQ(a[idx], 0xEDA);
    }
}

TEST(ConstructTest, Copy) {
    nostd::Array<int> a(10, 0xEDA);
    auto copy(a);

    TestEqual(a, copy);
}

TEST(ConstructTest, Move) {
    nostd::Array<int> a(10, 0xEDA);

    TestMove(a);
}

TEST(ConstructTest, Assign) {
    int const N = 500;
    nostd::Array<int> a;
    for (int i = 0; i != N; ++i) a.push_back(2 * i + 1);

    for (int i = 0; i != N; ++i) EXPECT_EQ(2 * i + 1, a[i]);

    nostd::Array<int> const& ca = a;

    for (int i = 0; i != N; ++i) EXPECT_EQ(2 * i + 1, ca[i]);

}

TEST(IteratorTest, Sort) {
    nostd::Array<int> a({2, 9, 1, 1, 1, 5, 3, 10, 1, 213, 24124});
    std::sort(a.begin(), a.end());

    for (size_t idx = 1; idx < a.size(); ++idx) {
        ASSERT_LE(a[idx-1], a[idx]);
    }
}

TEST(IteratorTest, Range) {
    nostd::Array<int> a({9, 5, 3, 10, 213});

    size_t idx = 0;
    for (auto& val: a) {
        ASSERT_EQ(a[idx], val);
        ++idx;
    }

    ASSERT_EQ(idx, a.size());
}

TEST(IteratorTest, Find) {
    nostd::Array<int> a({9, 5, 3, 10, 213});
    auto it = std::find(a.begin(), a.end(), 10);
    ASSERT_EQ(*it, 10);
}

TEST(MethodsTest, PushBack) {
    size_t const N = 5000;
    {
        nostd::Array<Tricky<size_t> > a;
        for (size_t i = 0; i != N; ++i)
            a.push_back(i);

        for (size_t i = 0; i != N; ++i)
            EXPECT_EQ(i, a[i]);
    }

    Tricky<size_t>::expect_no_instances();
}

TEST(MethodsTest, PushBackSelf) {
    size_t const N = 500;
    {
        nostd::Array<Tricky<size_t> > a;
        a.push_back(42);
        for (size_t i = 0; i != N; ++i) a.push_back(a[0]);

        for (size_t i = 0; i != a.size(); ++i) EXPECT_EQ(42, a[i]);
    }

    Tricky<size_t>::expect_no_instances();
}

TEST(MethodsTest, Data) {
    size_t const N = 500;
    nostd::Array<Tricky<size_t> > a;

    for (size_t i = 0; i != N; ++i) a.push_back(2 * i + 1);

    {
        Tricky<size_t>* ptr = a.data();
        for (size_t i = 0; i != N; ++i) EXPECT_EQ(2 * i + 1, ptr[i]);
    }

    {
        Tricky<size_t> const* cptr = as_const(a).data();
        for (size_t i = 0; i != N; ++i) EXPECT_EQ(2 * i + 1, cptr[i]);
    }
}

TEST(MethodsTest, FrontBack) {
    size_t const N = 500;
    nostd::Array<Tricky<size_t> > a;
    for (size_t i = 0; i != N; ++i) a.push_back(2 * i + 1);

    EXPECT_EQ(1, a.front());
    EXPECT_EQ(1, as_const(a).front());

    EXPECT_EQ(999, a.back());
    EXPECT_EQ(999, as_const(a).back());
}

TEST(MethodsTest, Capacity) {
    size_t const N = 500;
    {
        nostd::Array<Tricky<size_t> > a;
        a.reserve(N);
        EXPECT_LE(N, a.capacity());
        for (size_t i = 0; i != N - 1; ++i) a.push_back(2 * i + 1);
        EXPECT_LE(N, a.capacity());
        a.shrink_to_fit();
        EXPECT_EQ(N - 1, a.capacity());
    }
    Tricky<size_t>::expect_no_instances();
}

TEST(MethodsTest, SuperfluousReserve) {
    size_t const N = 500, K = 100;
    {
        nostd::Array<Tricky<size_t> > a;
        a.reserve(N);
        EXPECT_GE(a.capacity(), N);
        a.reserve(K);
        EXPECT_GE(a.capacity(), N);
    }
    Tricky<size_t>::expect_no_instances();
}

TEST(MethodsTest, Clear) {
    size_t const N = 500;
    {
        nostd::Array<Tricky<size_t> > a;
        for (size_t i = 0; i != N; ++i) a.push_back(2 * i + 1);
        size_t c = a.capacity();
        a.clear();
        EXPECT_EQ(c, a.capacity());
    }
    Tricky<size_t>::expect_no_instances();
}

TEST(MethodsTest, SuperfluousShrinkToFit) {
    size_t const N = 500;
    {
        nostd::Array<Tricky<size_t> > a;
        a.reserve(N);
        size_t c = a.capacity();
        for (size_t i = 0; i != c; ++i) a.push_back(2 * i + 1);
        Tricky<size_t>* old_data = a.data();
        a.shrink_to_fit();
        EXPECT_EQ(old_data, a.data());
    }
    Tricky<size_t>::expect_no_instances();
}

TEST(MethodsTest, SelfAssign) {
    size_t const N = 500;
    {
        nostd::Array<Tricky<size_t> > a;
        for (size_t i = 0; i != N; ++i) a.push_back(2 * i + 1);
        a = a;

        for (size_t i = 0; i != N; ++i) EXPECT_EQ(2 * i + 1, a[i]);
    }
    Tricky<size_t>::expect_no_instances();
}

TEST(MethodsTest, PopBack) {
    size_t const N = 500;
    nostd::Array<Tricky<size_t>> a;

    for (size_t i = 0; i != N; ++i) a.push_back(2 * i + 1);

    for (size_t i = N; i != 0; --i) {
        EXPECT_EQ(2 * i - 1, a.back());
        EXPECT_EQ(i, a.size());
        a.pop_back();
    }
    EXPECT_TRUE(a.empty());
    Tricky<size_t>::expect_no_instances();
}

// ---------------------------------------------------

TEST(Bool, DefaultConstruct) {
    nostd::Array<bool> a;
    EXPECT_TRUE(a.empty());
    EXPECT_EQ(0, a.size());
}

TEST(Bool, SizeConstruct) {
    nostd::Array<bool> a(10);
    ASSERT_EQ(a.size(), 10);
}

TEST(Bool, InitList) {
    nostd::Array<bool> a({true, true, true});
    ASSERT_EQ(a.size(), 3);
    for (size_t idx = 0; idx < a.size(); ++idx) {
        ASSERT_EQ(a[idx], true);
    }
}

TEST(Bool, SizeVal) {
    nostd::Array<int> a(10, true);
    ASSERT_EQ(a.size(), 10);
    for (size_t idx = 0; idx < a.size(); ++idx) {
        ASSERT_EQ(a[idx], true);
    }
}

TEST(Bool, Copy) {
    nostd::Array<int> a(10, true);
    a.push_back(false);
    auto copy(a);

    TestEqual(a, copy);
}

TEST(Bool, Assign) {
    int const N = 500;
    nostd::Array<int> a;
    for (int i = 0; i != N; ++i) a.push_back(true);

    for (int i = 0; i != N; ++i) EXPECT_EQ(true, a[i]);

    auto ca = a;

    for (int i = 0; i != N; ++i) EXPECT_EQ(true, ca[i]);

}

TEST(Bool, Range) {
    nostd::Array<bool> a({true, false, true, false, false});

    size_t idx = 0;
    for (auto val: a) {
        ASSERT_EQ(a[idx], val);
        ++idx;
    }

    ASSERT_EQ(idx, a.size());
}

TEST(Bool, Find) {
    nostd::Array<bool> a({true, false, true, true, false});
    auto it = std::find(a.begin(), a.end(), true);
    ASSERT_EQ(*it, true);
}

TEST(Bool, PushBack) {
    size_t const N = 20;
    {
        nostd::Array<bool> a;
        for (size_t i = 0; i != N; ++i) {
            a.push_back(true);
        }

        for (size_t i = 0; i != N; ++i)
            EXPECT_TRUE(true == a[i]);
    }
}

TEST(Bool, PushBackSelf) {
    size_t const N = 500;
    {
        nostd::Array<bool> a;
        a.push_back(true);
        for (size_t i = 0; i != N; ++i) a.push_back(a[0]);

        for (size_t i = 0; i != a.size(); ++i) EXPECT_EQ(true, a[i]);
    }
}

TEST(Bool, FrontBack) {
    size_t const N = 500;
    nostd::Array<bool> a;
    for (size_t i = 0; i != N; ++i) a.push_back(true);

    EXPECT_EQ(true, a.front());
    EXPECT_EQ(true, as_const(a).front());

    EXPECT_EQ(true, a.back());
    EXPECT_EQ(true, as_const(a).back());
}

TEST(Bool, Capacity) {
    size_t const N = 500;
    {
        nostd::Array<bool> a;
        a.reserve(N);
        EXPECT_LE(N, a.capacity());
        for (size_t i = 0; i != N - 1; ++i) a.push_back(true);
        EXPECT_LE(N, a.capacity());
    }
}

TEST(Bool, SuperfluousReserve) {
    size_t const N = 500, K = 100;
    {
        nostd::Array<bool> a;
        a.reserve(N);
        EXPECT_GE(a.capacity(), N);
        a.reserve(K);
        EXPECT_GE(a.capacity(), N);
    }
}

TEST(Bool, Clear) {
    size_t const N = 500;
    {
        nostd::Array<bool> a;
        for (size_t i = 0; i != N; ++i) a.push_back(true);
        size_t c = a.capacity();
        a.clear();
        EXPECT_EQ(c, a.capacity());
    }
}

TEST(Bool, SuperfluousShrinkToFit) {
    size_t const N = 500;
    {
        nostd::Array<bool> a;
        a.reserve(N);
        size_t c = a.capacity();
        for (size_t i = 0; i != c; ++i) a.push_back(true);
        a.shrink_to_fit();
    }
}

TEST(Bool, SelfAssign) {
    size_t const N = 500;
    {
        nostd::Array<bool> a;
        for (size_t i = 0; i != N; ++i) a.push_back(true);
        a = a;

        for (size_t i = 0; i != N; ++i) EXPECT_EQ(true, a[i]);
    }
}

TEST(Bool, PopBack) {
    size_t const N = 500;
    nostd::Array<Tricky<bool>> a;

    for (size_t i = 0; i != N; ++i) a.push_back(true);

    for (size_t i = N; i != 0; --i) {
        EXPECT_EQ(true, a.back());
        EXPECT_EQ(i, a.size());
        a.pop_back();
    }
    EXPECT_TRUE(a.empty());
}

