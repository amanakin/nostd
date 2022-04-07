#include <gtest/gtest.h>

#include <nostd/storage/storage.h>

/* TODO
 1) Compile test - why there's no in google test :(

*/

TEST(LocalStorage, Construct) {
    nostd::storage::LocalStorage<int, 10> storage;
    ASSERT_EQ(storage.capacity(), 10);
}

TEST(LocalStorage, DataAccess) {
    nostd::storage::LocalStorage<int, 10> storage;
    for (size_t idx = 0; idx < storage.capacity(); ++idx) {
        storage.data(idx) = static_cast<int>(idx * 2);
    }

    for (size_t idx = 0; idx < storage.capacity(); ++idx) {
        ASSERT_EQ(storage.data(idx), idx * 2);
    }
}

TEST(LocalStorage, AccessException) {
    nostd::storage::LocalStorage<int, 1> storage;

    try {
        storage.data(1) = 0xBAD;
        FAIL() << "Expected exception throw";
    } catch (const std::out_of_range& err) {
        SUCCEED() << "Got out_of_range exception";
    } catch (...) {
        FAIL() << "Expected out_of_range exception";
    }
}

TEST(LocalStorage, Copy) {
    nostd::storage::LocalStorage<int, 1> storage1;
    storage1.data(0) = 0xDAD;

    nostd::storage::LocalStorage<int, 1> storage2(storage1);
    ASSERT_EQ(storage2.data(0), 0xDAD);
}

TEST(LocalStorage, Move) {
    nostd::storage::LocalStorage<int, 1> storage1;
    storage1.data(0) = 0xDAD;

    nostd::storage::LocalStorage<int, 1> storage2(std::move(storage1));
    ASSERT_EQ(storage2.data(0), 0xDAD);
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
    for (size_t idx = 0; idx < storage.capacity(); ++idx) {
        storage.data(idx) = static_cast<int>(idx * 2);
    }

    for (size_t idx = 0; idx < storage.capacity(); ++idx) {
        ASSERT_EQ(storage.data(idx), idx * 2);
    }
}