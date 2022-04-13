#include <gtest/gtest.h>

#include <nostd/array/array.h>
#include <nostd/storage/storage.h>

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

TEST(ConstructTest, DefaultConstruct) {
    nostd::Array<int, nostd::storage::DynamicStorage> array;
    ASSERT_EQ(array.size(), 0);
}

TEST(ConstructTest, SizeConstruct) {
    nostd::Array<int, nostd::storage::DynamicStorage> array(10);
    ASSERT_EQ(array.size(), 10);

    TestAccess(array);
}

TEST(ConstructTest, InitList) {
    nostd::Array<int, nostd::storage::DynamicStorage> array({0xEDA, 0xEDA, 0xEDA});
    ASSERT_EQ(array.size(), 3);
    for (size_t idx = 0; idx < array.size(); ++idx) {
        ASSERT_EQ(array[idx], 0xEDA);
    }
}

TEST(ConstructTest, SizeVal) {
    nostd::Array<int, nostd::storage::DynamicStorage> array(10, 0xEDA);
    ASSERT_EQ(array.size(), 10);
    for (size_t idx = 0; idx < array.size(); ++idx) {
        ASSERT_EQ(array[idx], 0xEDA);
    }
}



TEST(IteratorTest, Sort) {
    nostd::Array<int, nostd::storage::DynamicStorage> array({2, 9, 1, 1, 1, 5, 3, 10, 1, 213, 24124});

    std::swap(*array.begin(), *(array.end()-2));

    for (auto& val: array) {
        std::cout << val << '\n';
    }

    std::cout << "=============\n";

    std::cout << *array.begin() << '\n';
    std::cout << *(array.end() - 1) << '\n';

    std::sort(array.begin(), array.end());

    for (auto& val: array) {
        std::cout << val << '\n';
    }

    for (size_t idx = 1; idx < array.size(); ++idx) {
        ASSERT_LE(array[idx-1], array[idx]);
    }
}

TEST(IteratorTest, Range) {
    nostd::Array<int, nostd::storage::DynamicStorage> array({9, 5, 3, 10, 213});

    for (auto& val: array) {
        std::cout << val << '\n';
    }
}

TEST(IteratorTest, Find) {
    nostd::Array<int, nostd::storage::DynamicStorage> array({9, 5, 3, 10, 213});
    auto it = std::find(array.begin(), array.end(), 10);
    if (it == array.end()) {
        std::cout << "not found\n";
    } else {
        std::cout << *it << '\n';
    }
}