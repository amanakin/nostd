#include <gtest/gtest.h>

#include <nostd/storage/storage.h>

TEST(ConstructTest, EmptyConstruct) {
    nostd::storage::LocalStorage<int, 10> storage;
    ASSERT_EQ(storage.capacity(), 10);
}