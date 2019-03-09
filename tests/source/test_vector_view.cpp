/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2019 Alexey Gruzdev
 */

#include "gtest/gtest.h"

#include <vector>

#include <yato/vector_view.h>
#include <yato/range.h>


TEST(Yato_VectorView, common)
{
    int a[10] = { 0 };

    yato::vector_view<int*> v1(&a[0], 10);
    EXPECT_EQ(10U, v1.max_size());
    yato::vector_view<int*> v2(v1);

    yato::vector_view<const int*> v3(&a[4], 5);
    EXPECT_EQ(5U, v3.max_size());
    v3 = v2;

    v2 = std::move(v1);
    yato::vector_view<const int*> v4(std::move(v3));
    EXPECT_EQ(10U, v4.max_size());
}

TEST(Yato_VectorView, assign)
{
    int a[10] = { 0 };
    yato::vector_view<int*> v1(&a[0], &a[10], 10);
    EXPECT_EQ(10U, v1.size());

    v1.assign(2, 42);
    EXPECT_EQ(2U, v1.size());
    EXPECT_EQ(42, a[0]);
    EXPECT_EQ(42, a[1]);
    EXPECT_FALSE(v1.empty());
}

TEST(Yato_VectorView, resize)
{
    int a[10] = { 0 };
    yato::vector_view<int*> v1(&a[0], 10);
    EXPECT_EQ(0U, v1.size());
    EXPECT_TRUE(v1.empty());
    v1.resize(2);
    EXPECT_EQ(2U, v1.size());
    EXPECT_FALSE(v1.empty());
    EXPECT_EQ(0, a[0]);
    EXPECT_EQ(0, a[1]);
    v1.resize(3, 42);
    EXPECT_EQ(3U, v1.size());
    EXPECT_FALSE(v1.empty());
    EXPECT_EQ(0, a[0]);
    EXPECT_EQ(0, a[1]);
    EXPECT_EQ(42, a[2]);
    a[0] = 43;
    v1.resize(1, -1);
    EXPECT_EQ(1U, v1.size());
    EXPECT_FALSE(v1.empty());
    EXPECT_EQ(43, a[0]);
    v1.resize(0);
    EXPECT_EQ(0U, v1.size());
    EXPECT_TRUE(v1.empty());

    //EXPECT_THROW(v1.resize(11), yato::assertion_error);

    EXPECT_NO_THROW(v1.resize(10));
}

TEST(Yato_VectorView, at)
{
    int a[10] = { 0 };
    yato::vector_view<int*> v1(&a[0], &a[1], 10);
    EXPECT_EQ(1U, v1.size());
    EXPECT_EQ(0, v1.at(0));
    EXPECT_EQ(0, v1[0]);
    EXPECT_FALSE(v1.empty());

    v1.assign(2, 42);
    EXPECT_EQ(2U, v1.size());
    EXPECT_EQ(42, v1.at(1));
    EXPECT_EQ(42, v1[1]);
    EXPECT_FALSE(v1.empty());

    EXPECT_THROW(v1.at(2), yato::out_of_range_error);

}

TEST(Yato_VectorView, range)
{
    int a[10] = { 0 };
    yato::vector_view<int*> v1(&a[0], &a[2], 10);
    EXPECT_EQ(2U, v1.size());
    EXPECT_EQ(0, v1[0]);
    EXPECT_EQ(0, v1[1]);

    for (auto & x : yato::make_range(v1)) {
        x = 42;
    }
    EXPECT_EQ(42, v1[0]);
    EXPECT_EQ(42, v1[1]);
}

TEST(Yato_VectorView, push_pop)
{
    int a[10] = { 0 };
    yato::vector_view<int*> v1(&a[0], 2);
    EXPECT_EQ(0U, v1.size());

    v1.push_back(1);
    EXPECT_EQ(1U, v1.size());
    v1.push_back(2);
    EXPECT_EQ(2U, v1.size());
    
    EXPECT_EQ(1, v1[0]);
    EXPECT_EQ(2, v1[1]);

    //EXPECT_THROW(v1.push_back(3), yato::assertion_error);

    v1.pop_back();
    EXPECT_EQ(1U, v1.size());
    v1.push_back(3);
    EXPECT_EQ(2U, v1.size());
    EXPECT_EQ(3, v1.at(1));
    v1.pop_back();
    EXPECT_EQ(1U, v1.size());
    v1.pop_back();
    EXPECT_EQ(0U, v1.size());
    EXPECT_EQ(true, v1.empty());

    //EXPECT_THROW(v1.pop_back(), yato::assertion_error);
}

TEST(Yato_VectorView, insert)
{
    int a[10] = { 0 };
    yato::vector_view<int*> v(&a[0], 3);
    v.resize(2);
    v[0] = 1;
    v[1] = 3;

    EXPECT_EQ(2U, v.size());
    EXPECT_EQ(1, v[0]);
    EXPECT_EQ(3, v[1]);

    auto it = v.insert(v.cbegin() + 1, 2);
    EXPECT_EQ(3U, v.size());
    EXPECT_EQ(1, v[0]);
    EXPECT_EQ(2, v[1]);
    EXPECT_EQ(3, v[2]);
    EXPECT_EQ(2, *it);

    //EXPECT_THROW(v.insert(v.begin(), 0), yato::assertion_error);

    yato::vector_view<int*> v2(std::begin(a), 10);
    v2.insert(v.begin(), { 10, 10, 10 });
    v2.insert(v.begin() + 1, { 20, 20, 20 });
    EXPECT_EQ(6U, v2.size());
    EXPECT_EQ(10, v2[0]);
    EXPECT_EQ(20, v2[1]);
    EXPECT_EQ(20, v2[2]);
    EXPECT_EQ(20, v2[3]);
    EXPECT_EQ(10, v2[4]);
    EXPECT_EQ(10, v2[5]);
}

TEST(Yato_VectorView, erase)
{
    int a[] = { 1, 2, 3, 4, 5, 6 };
    yato::vector_view<int*> v(std::begin(a), std::end(a), 6);
    v.erase(v.cbegin() + 1, v.cbegin() + 3);
    EXPECT_EQ(4U, v.size());
    EXPECT_EQ(1, v[0]);
    EXPECT_EQ(4, v[1]);
    EXPECT_EQ(5, v[2]);
    EXPECT_EQ(6, v[3]);
}
