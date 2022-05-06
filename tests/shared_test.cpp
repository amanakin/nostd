#include <nostd/pointers/shared_ptr.h>

#include <gtest/gtest.h>


#include "test_util.h"

TEST(SharedPtr, Construct) {
    nostd::SharedPtr<Tricky<int>> shared;

    ASSERT_EQ(shared.get(), nullptr);
    ASSERT_EQ(shared.use_count(), 0);
}

TEST(SharedPtr, ConstructNull) {
    nostd::SharedPtr<Tricky<int>> shared(nullptr);

    ASSERT_EQ(shared.get(), nullptr);
    ASSERT_EQ(shared.use_count(), 0);
}

TEST(SharedPtr, CopyMove) {
    {
        nostd::SharedPtr<Tricky<int>> shared(new Tricky<int>);
        ASSERT_EQ(shared.use_count(), 1);
        auto shared2(std::move(shared));
        ASSERT_EQ(shared2.use_count(), 1);
    }
    Tricky<int>::expect_no_instances();

}

TEST(SharedPtr, CopyMoveAssign) {
    {
        nostd::SharedPtr<Tricky<int>> shared(new Tricky<int>);
        decltype(shared) shared2(new Tricky<int>(5));
        auto shared3(shared2);
        ASSERT_EQ(shared.use_count(), 1);
        ASSERT_EQ(shared3.use_count(), 2);
        shared2 = std::move(shared);

        ASSERT_EQ(shared2.use_count(), 1);
        ASSERT_EQ(shared3.use_count(), 1);
        ASSERT_TRUE(shared2 != shared3);
    }
    Tricky<int>::expect_no_instances();
}

TEST(SharedPtr, Reset) {
    {
        nostd::SharedPtr<Tricky<int>> shared(new Tricky<int>);
        shared.reset();

        shared = nostd::MakeShared<Tricky<int>>(2);
        auto shared2 = shared;
        ASSERT_EQ(shared.use_count(), 2);
        ASSERT_EQ(*shared, 2);

        shared2.reset(new Tricky<int>(3));
        ASSERT_EQ(*shared, 2);
        ASSERT_EQ(*shared2, 3);
    }
    Tricky<int>::expect_no_instances();
}

TEST(SharedPtr, MakeShared) {
    {
        auto shared = nostd::MakeShared<Tricky<int>>(1);
        auto shared2(shared); // NOLINT

        ASSERT_EQ(shared2->get(), 1);
        ASSERT_TRUE(shared == shared2);
    }
    Tricky<int>::expect_no_instances();
}

TEST(SharedPtr, Compare) {
    {
        auto shared = nostd::MakeShared<Tricky<int>>(1);
        ASSERT_TRUE(*shared == 1);

        shared.reset();

        ASSERT_TRUE(shared == nullptr);
        if (shared) {
            FAIL() << "shared true when expected false";
        }

        shared.reset(new Tricky<int>(2));
        ASSERT_TRUE(shared != nullptr);
        ASSERT_FALSE(shared == nullptr);
        ASSERT_TRUE(*shared == 2);
    }
    Tricky<int>::expect_no_instances();
}