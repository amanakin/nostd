#include <gtest/gtest.h>

#include <nostd/array/array.h>
#include <nostd/storage/storage.h>

TEST(ConstructTest, EmptyConstruct) {
    nostd::Array<int, nostd::storage::DynamicStorage> array;
    ASSERT_EQ(array.size(), 0);
}