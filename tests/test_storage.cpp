#include <gtest/gtest.h>

#include <nostd/storage/storage.h>

/* TODO
    * Complete TestAssign
    * Compile test - why there's no in google test :(
    *
*/

template <typename Storage>
void TestAccess(Storage& storage) {
    for (size_t idx = 0; idx < storage.capacity(); ++idx) {
        storage.data(idx) = static_cast<int>(idx * 2);
    }

    for (size_t idx = 0; idx < storage.capacity(); ++idx) {
        ASSERT_EQ(storage.data(idx), idx * 2);
    }
};

template <typename Storage>
void TestAccessException(const Storage& storage) {
    try {
        auto bad = storage.data(1);
        FAIL() << "Expected exception throw";
    } catch (const std::out_of_range& err) {
        SUCCEED() << "Got out_of_range exception";
    } catch (...) {
        FAIL() << "Expected out_of_range exception";
    }
}

template <typename Storage>
void TestEqual(const Storage& lhs, const Storage& rhs) {
    ASSERT_EQ(lhs.capacity(), rhs.capacity());
    for (size_t idx = 0; idx < lhs.capacity(); ++idx) {
        ASSERT_EQ(lhs.data(idx), rhs.data(idx));
    }
}

template <typename Storage>
void TestCopy(const Storage& storage) {
    auto copy(storage);

    TestEqual(storage, copy);
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
// ============================================================================

TEST(LocalStorage, Construct) {
    nostd::storage::LocalStorage<int, 10> storage;
    ASSERT_EQ(storage.capacity(), 10);
}

TEST(LocalStorage, DataAccess) {
    nostd::storage::LocalStorage<int, 10> storage;
    TestAccess(storage);
}

TEST(LocalStorage, AccessException) {
    nostd::storage::LocalStorage<int, 1> storage;
    TestAccessException(storage);
}

TEST(LocalStorage, Copy) {
    nostd::storage::LocalStorage<int, 1> storage;
    storage.data(0) = 0xDAD;

    TestCopy(storage);
}

TEST(LocalStorage, Move) {
    nostd::storage::LocalStorage<int, 1> storage;
    storage.data(0) = 0xDAD;

    TestMove(storage);
}

TEST(LocalStorage, Assign) {
    nostd::storage::LocalStorage<int, 1> storage1;
    storage1.data(0) = 0xDAD;

    auto storage2 = storage1;
    ASSERT_EQ(storage2.data(0), 0xDAD);

    auto storage3 = std::move(storage2);
    ASSERT_EQ(storage3.data(0), 0xDAD);
}

// ============================================================================

TEST(DynamicStorage, Construct) {
    nostd::storage::DynamicStorage<int> storage1;
    ASSERT_EQ(storage1.capacity(), 0);

    nostd::storage::DynamicStorage<int> storage2(10);
    ASSERT_EQ(storage2.capacity(), 10);
}

TEST(DynamicStorage, DataAccess) {
    nostd::storage::DynamicStorage<int> storage(10);
    TestAccess(storage);
}

TEST(DynamicStorage, AccessException) {
    nostd::storage::DynamicStorage<int> storage(1);
    TestAccessException(storage);
}

TEST(DynamicStorage, Resize) {
    nostd::storage::DynamicStorage<int> storage(2);
    storage.data(0) = 0xDAD;
    storage.data(1) = 0xBEB;

    storage.resize(100);
    ASSERT_EQ(storage.capacity(), 100);
    ASSERT_EQ(storage.data(0), 0xDAD);
    ASSERT_EQ(storage.data(1),  0xBEB);

    storage.resize(1);
    ASSERT_EQ(storage.capacity(), 1);
    ASSERT_EQ(storage.data(0), 0xDAD);
}

TEST(DynamicStorage, Copy) {
    nostd::storage::DynamicStorage<int> storage(1);
    storage.data(0) = 0xDAD;

    TestCopy(storage);
}

TEST(DynamicStorage, Move) {
    nostd::storage::DynamicStorage<int> storage(1);
    storage.data(0) = 0xDAD;

    TestMove(storage);
}

TEST(DynamicStorage, Assign) {
    nostd::storage::DynamicStorage<int> storage1(1);
    storage1.data(0) = 0xDAD;
    nostd::storage::DynamicStorage<int> storage2(1);
    storage2.data(0) = 0xBEB;

    TestAssign(storage1, storage2);
}

// ============================================================================