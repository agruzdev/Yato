/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2020 Alexey Gruzdev
 */

#include "gtest/gtest.h"

#include <yato/dynamic_array.h>

TEST(Yato_DynamicArray, common)
{
    yato::dynamic_array<int> a1;
    yato::dynamic_array<int> a2(4, 42);
    
    a1 = std::move(a2);

    yato::dynamic_array<int> a3(std::move(a1));

    EXPECT_EQ(42, a3[0]);
    EXPECT_EQ(42, a3[1]);
    EXPECT_EQ(42, a3.at(2));
    EXPECT_EQ(42, a3.at(3));

    EXPECT_EQ(42, a3.front());
    EXPECT_EQ(42, a3.back());

    yato::dynamic_array<int> a4 = { 1, 2, 3 };
    EXPECT_EQ(1, a4[0]);
    EXPECT_EQ(2, a4[1]);
    EXPECT_EQ(3, a4[2]);

    using std::swap;
    swap(a1, a2);


    EXPECT_TRUE(true);
}

namespace
{
    template <typename _T>
    class mean_alloc
        : public std::allocator<_T>
    {
    public:
        mean_alloc() {}

        mean_alloc(const mean_alloc & other)
            :std::allocator<_T>(other)
        { }

        mean_alloc(mean_alloc &&) = delete;
        mean_alloc& operator=(mean_alloc &&) = delete;
    };
}

TEST(Yato_DynamicArray, mean_alloc)
{
    yato::dynamic_array<int, mean_alloc<int>> a1;
    yato::dynamic_array<int, mean_alloc<int>> a2(4, 42);

    //a1 = std::move(a2);
    //yato::dynamic_array<int, mean_alloc<int>> a3(std::move(a1));

    EXPECT_TRUE(true);
}

namespace
{
    class Foo
    {
    public:
        mutable uint32_t x;
        Foo() : x(0) { }
        Foo(const Foo & other) { ++other.x; }

        Foo& operator=(Foo&) = delete;
    };
}

TEST(Yato_DynamicArray, assign)
{
    Foo f;
    EXPECT_EQ(0U, f.x);
    yato::dynamic_array<Foo> a1(3, f);
    EXPECT_EQ(3U, f.x);
    a1.assign(2, f);
    EXPECT_EQ(5U, f.x);
}

