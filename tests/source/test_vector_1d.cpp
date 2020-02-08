/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2020 Alexey Gruzdev
 */

#include "gtest/gtest.h"

#include <memory>
#include <algorithm>
#include <numeric>
#include <initializer_list>

#include <yato/vector_nd.h>

namespace
{
    class FooCounted
    {
        int m_val;

    public:
        static size_t ctors;
        static size_t dtors;

        static void reset_counters()
        {
            ctors = 0;
            dtors = 0;
        }

        FooCounted()
            : m_val(42)
        {
            ++ctors;
        }

        FooCounted(int v)
            : m_val(v)
        {
            ++ctors;
        }

        FooCounted(const FooCounted & other)
            : m_val(other.m_val)
        {
            ++ctors;
        }

        FooCounted(FooCounted && other) noexcept
            : m_val(other.m_val)
        {
            ++ctors;
        }

        ~FooCounted()
        {
            ++dtors;
        }

        FooCounted& operator= (const FooCounted & other)
        {
            m_val = other.m_val;
            return *this;
        }
        
        FooCounted& operator= (FooCounted && other) noexcept
        {
            m_val = other.m_val;
            return *this;
        }

        operator int() const
        {
            return m_val;
        }
    };

    size_t FooCounted::ctors = 0;
    size_t FooCounted::dtors = 0;
}

TEST(Yato_Vector1D, common)
{
    class A {
        int m_val;
    public:
        A(int x): m_val(x) {};

        int get() const { return m_val; }
    };

    try {
        yato::vector_1d<int> vec0{};
        EXPECT_TRUE(vec0.empty());
        EXPECT_EQ(1u, vec0.dimensions_num());
        EXPECT_EQ(0u, vec0.size(0));
        EXPECT_EQ(0u, vec0.size());
        EXPECT_EQ(0u, vec0.total_size());

        yato::vector_1d<int> vec1(yato::dims(3));
        EXPECT_FALSE(vec1.empty());
        EXPECT_EQ(1u, vec1.dimensions_num());
        EXPECT_EQ(3u, vec1.size(0));
        EXPECT_EQ(3u, vec1.size());
        EXPECT_EQ(3u, vec1.total_size());

        EXPECT_EQ(0u, yato::length(vec0));
        EXPECT_EQ(3u, yato::length(vec1));

        yato::vector_1d<float> vec3(yato::dims(2), 11.0f);

        yato::vector_1d<int> vec4 = { 1, 1, 1, 2, 2, 2 };
        yato::vector_1d<int> vec5 = { 5 };
        yato::vector_1d<int> vec6 = { };

        EXPECT_EQ(6u, yato::length(vec4));
        EXPECT_EQ(1u, yato::length(vec5));
        EXPECT_EQ(0u, yato::length(vec6));

        yato::vector_1d<A> vec7 = { A(1) };
        EXPECT_EQ(1u, yato::length(vec7));

        std::vector<int> sizes;
        sizes.push_back(5);
        const yato::vector_1d<int> vec8(yato::make_range(sizes.cbegin(), sizes.cend()), 1);
        EXPECT_EQ(5u, yato::length(vec8));
    }
    catch (...)
    {
        EXPECT_TRUE(false);
    }
    EXPECT_TRUE(true);
}

TEST(Yato_Vector1D, common_usertype)
{
    FooCounted::reset_counters();
    try {
        yato::vector_1d<FooCounted> vec0{};
        EXPECT_TRUE(vec0.empty());
        EXPECT_EQ(1u, vec0.dimensions_num());
        EXPECT_EQ(0u, vec0.size(0));
        EXPECT_EQ(0u, vec0.size());
        EXPECT_EQ(0u, vec0.total_size());

        yato::vector_1d<FooCounted> vec1(yato::dims(3));
        EXPECT_FALSE(vec1.empty());
        EXPECT_EQ(1u, vec1.dimensions_num());
        EXPECT_EQ(3u, vec1.size(0));
        EXPECT_EQ(3u, vec1.size());
        EXPECT_EQ(3u, vec1.total_size());

        EXPECT_EQ(0u, yato::length(vec0));
        EXPECT_EQ(3u, yato::length(vec1));

        yato::vector_1d<FooCounted> vec3(yato::dims(2), 11);

        yato::vector_1d<FooCounted> vec4 = { 1, 1, 1, 2, 2, 2 };
        yato::vector_1d<FooCounted> vec5 = { 5 };
        yato::vector_1d<FooCounted> vec6 = { };

        EXPECT_EQ(6u, yato::length(vec4));
        EXPECT_EQ(1u, yato::length(vec5));
        EXPECT_EQ(0u, yato::length(vec6));

        yato::vector_1d<FooCounted> vec7 = { FooCounted(1) };
        EXPECT_EQ(1u, yato::length(vec7));

        std::vector<FooCounted> sizes;
        sizes.push_back(5);
        const yato::vector_1d<FooCounted> vec8(yato::make_range(sizes.cbegin(), sizes.cend()), 1);
        EXPECT_EQ(5u, yato::length(vec8));
    }
    catch (...)
    {
        EXPECT_TRUE(false);
    }
    EXPECT_EQ(FooCounted::ctors, FooCounted::dtors);
}

TEST(Yato_Vector1D, common_2)
{
    yato::vector_1d<int> vec1 = { 1, 1, 1, 2, 2, 2 };
    yato::vector_1d<int> vec2(vec1);

    EXPECT_EQ(yato::length(vec1), yato::length(vec2));
    EXPECT_EQ(6u, yato::length(vec1));

    yato::vector_1d<int> vec3(std::move(vec1));
    EXPECT_EQ(yato::length(vec3), yato::length(vec2));
    EXPECT_EQ(0u, yato::length(vec1));
}

TEST(Yato_Vector1D, ctor_from_range)
{
    int raw[] = { 1, 2, 3, 4, 5, 6 };

    auto v1 = yato::vector_1d<int>(yato::dims(6), std::begin(raw), std::end(raw));
    EXPECT_EQ(6u, v1.size(0));
    EXPECT_EQ(1, v1[0]);
    EXPECT_EQ(2, v1[1]);
    EXPECT_EQ(3, v1[2]);
    EXPECT_EQ(4, v1[3]);
    EXPECT_EQ(5, v1[4]);
    EXPECT_EQ(6, v1[5]);
}

TEST(Yato_Vector1D, construct_from_initializer_list)
{
    FooCounted::reset_counters();
    {
        yato::vector_1d<FooCounted> vec0 = {};
        EXPECT_EQ(0u, vec0.total_size());

        yato::vector_1d<FooCounted> vec1 = { 1, 1, 1, 2, 2, 2 };
        EXPECT_EQ(6u, vec1.total_size());
        EXPECT_EQ(1, vec1[0]);
        EXPECT_EQ(1, vec1[1]);
        EXPECT_EQ(1, vec1[2]);
        EXPECT_EQ(2, vec1[3]);
        EXPECT_EQ(2, vec1[4]);
        EXPECT_EQ(2, vec1[5]);
    }
    EXPECT_EQ(FooCounted::ctors, FooCounted::dtors);
}

TEST(Yato_Vector1D, access)
{
    yato::vector_1d<int> vec1 = { 1, 2, 3, 4 };
    EXPECT_TRUE(vec1[0] == 1);
    EXPECT_TRUE(vec1[1] == 2);
    EXPECT_TRUE(vec1[2] == 3);
    EXPECT_TRUE(vec1[3] == 4);
}

TEST(Yato_Vector1D, access_1)
{
    const size_t size = static_cast<uint8_t>(std::rand() % 101);

    short val = static_cast<short>((std::rand() % 1000) / 10.0f);
    yato::vector_1d<short> vec_1d(size, val);

    for (size_t i = 0; i < size; ++i) {
        short x{0};
        EXPECT_NO_THROW(x = vec_1d[i]);
        EXPECT_EQ(val, x);
    }
}

TEST(Yato_Vector1D, access_view)
{
    yato::vector_1d<int> vec1 = { 1, 2, 3, 4 };

    auto view1 = vec1.cview();
    EXPECT_TRUE(view1[0] == 1);
    EXPECT_TRUE(view1[1] == 2);
    EXPECT_TRUE(view1[2] == 3);
    EXPECT_TRUE(view1[3] == 4);

    auto view2 = vec1.view();
    view2[1] = 14;
    view2.at(3) = 24;

    EXPECT_TRUE(view1[0] == 1);
    EXPECT_TRUE(view1[1] == 14);
    EXPECT_TRUE(view1[2] == 3);
    EXPECT_TRUE(view1[3] == 24);
}

TEST(Yato_Vector1D, access_view_1)
{
    const size_t size = static_cast<uint8_t>(std::rand() % 101);

    short val = static_cast<short>((std::rand() % 1000) / 10.0f);
    yato::vector_1d<short> vec_1d(size, val);
    auto view_1d = vec_1d.cview();

    for (size_t i = 0; i < size; ++i) {
        short x{0};
        EXPECT_NO_THROW(x = view_1d[i]);
        EXPECT_EQ(val, x);
    }
}

TEST(Yato_Vector1D, method_at)
{
    yato::vector_1d<int> vec1 = { 1, 2, 3, 4 };
    EXPECT_TRUE(vec1.at(0) == 1);
    EXPECT_TRUE(vec1.at(1) == 2);
    EXPECT_TRUE(vec1.at(2) == 3);
    EXPECT_TRUE(vec1.at(3) == 4);
    
    EXPECT_THROW(vec1.at(4),  yato::out_of_range_error);
    EXPECT_THROW(vec1.at(5),  yato::out_of_range_error);
    EXPECT_THROW(vec1.at(11), yato::out_of_range_error);
}

TEST(Yato_Vector1D, method_at_1)
{
    const size_t size = static_cast<uint8_t>(std::rand() % 101);

    short val = static_cast<short>((std::rand() % 1000) / 10.0f);
    yato::vector_1d<short> vec_1d(size, val);

    for (size_t i = 0; i < size; ++i) {
        short x{0};
        EXPECT_THROW(x = vec_1d.at(size + i / 2), yato::out_of_range_error);
        EXPECT_NO_THROW(x = vec_1d.at(i));
        EXPECT_EQ(val, x);
    }
}

TEST(Yato_Vector1D, reserve)
{
    yato::vector_1d<int> vec;
    EXPECT_EQ(0u, vec.capacity());

    vec.reserve(100);
    EXPECT_EQ(100u, vec.capacity());

    vec.reserve(10);
    EXPECT_EQ(100u, vec.capacity());

    vec.reserve(0);
    EXPECT_EQ(100u, vec.capacity());

    vec.reserve(101);
    EXPECT_EQ(101u, vec.capacity());

    vec.shrink_to_fit();
    EXPECT_EQ(0u, vec.capacity());
}

TEST(Yato_Vector1D, reserve_usertype)
{
    FooCounted::reset_counters();
    {
        yato::vector_1d<int> vec;
        EXPECT_EQ(0u, vec.capacity());

        vec.reserve(100);
        EXPECT_EQ(100u, vec.capacity());

        vec.reserve(10);
        EXPECT_EQ(100u, vec.capacity());

        vec.reserve(0);
        EXPECT_EQ(100u, vec.capacity());

        vec.reserve(101);
        EXPECT_EQ(101u, vec.capacity());

        vec.shrink_to_fit();
        EXPECT_EQ(0u, vec.capacity());
    }
    EXPECT_EQ(FooCounted::ctors, FooCounted::dtors);
}

TEST(Yato_Vector1D, assign)
{
    yato::vector_1d<int> vec = {};
    
    EXPECT_TRUE(vec.empty());
    for (const int & x : vec.plain_crange()) {
        (void)x;
        EXPECT_TRUE(false);
    }

    vec.assign(yato::dims(4), 1);
    
    EXPECT_FALSE(vec.empty());
    for (const int & x : vec.plain_crange()) {
        EXPECT_EQ(1, x);
    }

    vec.assign(yato::dims(1), 42);

    EXPECT_FALSE(vec.empty());
    for (const int & x : vec.plain_crange()) {
        EXPECT_EQ(42, x);
    }

    vec.assign(yato::dims(3), 10);

    EXPECT_FALSE(vec.empty());
    for (const int & x : vec.plain_crange()) {
        EXPECT_EQ(10, x);
    }
}

TEST(Yato_Vector1D, assign_usertype)
{
    FooCounted::reset_counters();
    {
        yato::vector_1d<FooCounted> vec = {};
    
        EXPECT_TRUE(vec.empty());
        for (const FooCounted & x : vec.plain_crange()) {
            (void)x;
            EXPECT_TRUE(false);
        }

        vec.assign(yato::dims(4), 1);
    
        EXPECT_FALSE(vec.empty());
        for (const FooCounted & x : vec.plain_crange()) {
            EXPECT_EQ(1, x);
        }

        vec.assign(yato::dims(1), 42);

        EXPECT_FALSE(vec.empty());
        for (const FooCounted & x : vec.plain_crange()) {
            EXPECT_EQ(42, x);
        }

        vec.assign(yato::dims(3), 10);

        EXPECT_FALSE(vec.empty());
        for (const FooCounted & x : vec.plain_crange()) {
            EXPECT_EQ(10, x);
        }
    }
    EXPECT_EQ(FooCounted::ctors, FooCounted::dtors);
}

TEST(Yato_Vector1D, iterator)
{
    yato::vector_1d<int> vec5 = { 1, 1, 1, 2, 1, 3 };

    auto first = vec5.cbegin();
    auto last  = vec5.cend();
    for (int i = 1; first != last; first += 2, ++i) {
        const int & x = *first;
        EXPECT_EQ(1, x);
        EXPECT_EQ(*(first + 1), i);
    }

    yato::vector_1d<int> vec6 = { 1, 1, 1, 0, 0, 0, 1, 1, 1 };
    auto dst = std::next(vec6.begin(), 3);
    std::fill(dst, dst + 3, 1);
    for (auto x : vec6.plain_range()) {
        EXPECT_EQ(1, x);
    }
}

TEST(Yato_Vector1D, push)
{
    yato::vector_1d<int> vec1;
    EXPECT_EQ(0u, vec1.size(0));
    vec1.push_back(1);
    EXPECT_EQ(1u, vec1.size(0));
    vec1.push_back(2);
    EXPECT_EQ(2u, vec1.size(0));
    vec1.push_back(3);
    EXPECT_EQ(3u, vec1.size(0));
    vec1.push_back(4);
    EXPECT_EQ(4u, vec1.size(0));

    EXPECT_EQ(1, vec1[0]);
    EXPECT_EQ(2, vec1[1]);
    EXPECT_EQ(3, vec1[2]);
    EXPECT_EQ(4, vec1[3]);

    yato::vector_1d<short> vec2;
    vec2.reserve(8);
    EXPECT_EQ(0u, vec2.size(0));
    vec2.push_back(1);
    EXPECT_EQ(1u, vec2.size(0));
    vec2.push_back(2);
    EXPECT_EQ(2u, vec2.size(0));
    vec2.push_back(3);
    EXPECT_EQ(3u, vec2.size(0));
    vec2.push_back(4);
    EXPECT_EQ(4u, vec2.size(0));

    EXPECT_EQ(1, vec2[0]);
    EXPECT_EQ(2, vec2[1]);
    EXPECT_EQ(3, vec2[2]);
    EXPECT_EQ(4, vec2[3]);
}

TEST(Yato_Vector1D, push_usertype)
{
    FooCounted::reset_counters();
    {
        yato::vector_1d<FooCounted> vec1;
        EXPECT_EQ(0u, vec1.size(0));
        vec1.push_back(1);
        EXPECT_EQ(1u, vec1.size(0));
        vec1.push_back(2);
        EXPECT_EQ(2u, vec1.size(0));
        vec1.emplace_back(3);
        EXPECT_EQ(3u, vec1.size(0));
        vec1.emplace_back(4);
        EXPECT_EQ(4u, vec1.size(0));

        EXPECT_EQ(1, vec1[0]);
        EXPECT_EQ(2, vec1[1]);
        EXPECT_EQ(3, vec1[2]);
        EXPECT_EQ(4, vec1[3]);

        yato::vector_1d<FooCounted> vec2;
        vec2.reserve(8);
        EXPECT_EQ(0u, vec2.size(0));
        vec2.push_back(1);
        EXPECT_EQ(1u, vec2.size(0));
        vec2.emplace_back(2);
        EXPECT_EQ(2u, vec2.size(0));
        vec2.emplace_back(3);
        EXPECT_EQ(3u, vec2.size(0));
        vec2.push_back(4);
        EXPECT_EQ(4u, vec2.size(0));

        EXPECT_EQ(1, vec2[0]);
        EXPECT_EQ(2, vec2[1]);
        EXPECT_EQ(3, vec2[2]);
        EXPECT_EQ(4, vec2[3]);
    }
    EXPECT_EQ(FooCounted::ctors, FooCounted::dtors);
}



namespace
{
    class FooMoving
    {
        int m_val;

    public:
        FooMoving(int x)
            : m_val(x)
        { }

        ~FooMoving() = default;

        FooMoving(const FooMoving& other)
            : m_val(other.m_val)
        { }

        FooMoving(FooMoving&& other) noexcept
            : m_val(other.m_val)
        {
            other.m_val = -1;
        }

        FooMoving& operator= (const FooMoving& other) = default;

        FooMoving& operator= (FooMoving&& other) noexcept
        {
            m_val = other.m_val;
            other.m_val = -1;
            return *this;
        }

        operator int() const
        {
            return m_val;
        }
    };
}

TEST(Yato_Vector1D, push_2)
{
    FooMoving v1{ 1 };
    FooMoving v2{ 2 };
    FooMoving v3{ 3 };
    FooMoving v4{ 4 };

    yato::vector_1d<FooMoving> vec1;
    EXPECT_EQ(0u, vec1.size(0));
    vec1.push_back(std::move(v1));
    EXPECT_EQ(1u, vec1.size(0));
    vec1.push_back(std::move(v2));
    EXPECT_EQ(2u, vec1.size(0));
    vec1.push_back(std::move(v3));
    EXPECT_EQ(3u, vec1.size(0));
    vec1.push_back(std::move(v4));
    EXPECT_EQ(4u, vec1.size(0));

    EXPECT_EQ(1, vec1[0]);
    EXPECT_EQ(2, vec1[1]);
    EXPECT_EQ(3, vec1[2]);
    EXPECT_EQ(4, vec1[3]);

    EXPECT_EQ(-1, v1);
    EXPECT_EQ(-1, v2);
    EXPECT_EQ(-1, v3);
    EXPECT_EQ(-1, v4);
}

TEST(Yato_Vector1D, push_pop)
{
    yato::vector_1d<int>  vec = { 1, 2, 3, 4 };
    vec.clear();
    EXPECT_TRUE(vec.empty());

    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(3);

    EXPECT_EQ(3u, vec.size());
    EXPECT_EQ(1, vec[0]);
    EXPECT_EQ(2, vec[1]);
    EXPECT_EQ(3, vec[2]);

    vec.push_back(4);
    EXPECT_EQ(4u, vec.size());
    EXPECT_EQ(1, vec[0]);
    EXPECT_EQ(2, vec[1]);
    EXPECT_EQ(3, vec[2]);
    EXPECT_EQ(4, vec[3]);

    vec.pop_back();
    EXPECT_EQ(3u, vec.size());
    EXPECT_EQ(1, vec[0]);
    EXPECT_EQ(2, vec[1]);
    EXPECT_EQ(3, vec[2]);
    
    vec.pop_back();
    EXPECT_EQ(2u, vec.size());
    EXPECT_EQ(1, vec[0]);
    EXPECT_EQ(2, vec[1]);

    vec.pop_back();
    EXPECT_EQ(1u, vec.size());
    EXPECT_EQ(1, vec[0]);

    vec.pop_back();
    EXPECT_TRUE(vec.empty());
}


TEST(Yato_Vector1D, common_3)
{
    std::vector<int> std_vec1 = { 1, 2, 3 };
    std::vector<int> std_vec2 = { 4, 5, 6 };

    yato::vector_nd<int, 1> vec1 = std_vec1;
    EXPECT_EQ(3U, vec1.size(0));
    EXPECT_EQ(1, vec1[0]);
    EXPECT_EQ(2, vec1[1]);
    EXPECT_EQ(3, vec1[2]);

    yato::vector_nd<int, 1> vec2 = std::move(std_vec2);
    EXPECT_EQ(3U, vec2.size(0));
    EXPECT_EQ(4, vec2[0]);
    EXPECT_EQ(5, vec2[1]);
    EXPECT_EQ(6, vec2[2]);

    vec1 = std::move(vec2);
    EXPECT_EQ(3U, vec1.size(0));
    EXPECT_EQ(4, vec1[0]);
    EXPECT_EQ(5, vec1[1]);
    EXPECT_EQ(6, vec1[2]);

    std_vec1 = vec1;
    EXPECT_EQ(3U, std_vec1.size());
    EXPECT_EQ(4, std_vec1[0]);
    EXPECT_EQ(5, std_vec1[1]);
    EXPECT_EQ(6, std_vec1[2]);

    std_vec2 = std::move(vec1);
    EXPECT_EQ(3U, std_vec2.size());
    EXPECT_EQ(4, std_vec2[0]);
    EXPECT_EQ(5, std_vec2[1]);
    EXPECT_EQ(6, std_vec2[2]);
}

TEST(Yato_Vector1D, insert)
{
    yato::vector_1d<int> vec = { 1, 1, 3, 3 };

    yato::vector_1d<short> line = { 2, 2 };
    vec.insert(std::next(vec.begin(), 2), line.cbegin(), line.cend());

    vec.insert(vec.begin(), 0);
    vec.insert(vec.begin(), 0);

    vec.insert(vec.end(), 2, 4);

    for(int i = 0; i < 10; i += 2) {
        int idx = i / 2;
        EXPECT_NO_THROW(
            EXPECT_EQ(idx, vec.at(i + 0))
        );
        EXPECT_NO_THROW(
            EXPECT_EQ(idx, vec.at(i + 1))
        );
    }
}

TEST(Yato_Vector1D, insert_usertype)
{
    FooCounted::reset_counters();
    {
        yato::vector_1d<FooCounted> vec = { 1, 1, 3, 3 };

        yato::vector_1d<FooCounted> line = { 2, 2 };
        vec.insert(std::next(vec.begin(), 2), line.cbegin(), line.cend());

        vec.insert(vec.begin(), 0);
        vec.insert(vec.begin(), 0);

        vec.insert(vec.end(), 2, FooCounted(4));

        for(int i = 0; i < 10; i += 2) {
            int idx = i / 2;
            EXPECT_NO_THROW(
                EXPECT_EQ(idx, vec.at(i + 0))
            );
            EXPECT_NO_THROW(
                EXPECT_EQ(idx, vec.at(i + 1))
            );
        }
    }
    EXPECT_EQ(FooCounted::ctors, FooCounted::dtors);
}

TEST(Yato_Vector1D, insert_move)
{
    yato::vector_1d<FooMoving> vec1 = { 1, 4 };
    yato::vector_1d<FooMoving> vec2 = { 2, 3 };

    vec1.insert(std::next(vec1.begin()), std::make_move_iterator(vec2.begin()), std::make_move_iterator(vec2.end()));

    int i = 1;
    for (const auto & val : vec1) {
        EXPECT_EQ(i, val);
        ++i;
    }
    for (const auto & val : vec2) {
        EXPECT_EQ(-1, val);
    }
}

TEST(Yato_Vector1D, insert_empty)
{
    yato::vector_1d<int> vec = { };

    yato::vector_1d<short> line = { 2, 2 };
    vec.insert(vec.begin(), std::begin(line), std::end(line));

    EXPECT_EQ(2u, vec.size(0));
    EXPECT_EQ(2, vec[0]);
    EXPECT_EQ(2, vec[1]);
}

TEST(Yato_Vector1D, insert_empty_2)
{
    yato::vector_1d<int> vec = { };
    vec.reserve(2);

    yato::vector_1d<short> line = { 2, 2 };
    vec.insert(vec.begin(), std::begin(line), std::end(line));

    EXPECT_EQ(2u, vec.size(0));
    EXPECT_EQ(2, vec[0]);
    EXPECT_EQ(2, vec[1]);
}

TEST(Yato_Vector1D, insert_empty_usertype)
{
    FooCounted::reset_counters();
    {
        yato::vector_1d<FooCounted> vec = { };

        yato::vector_1d<FooCounted> line = { 2, 2 };
        vec.insert(vec.begin(), std::begin(line), std::end(line));

        EXPECT_EQ(2u, vec.size(0));
        EXPECT_EQ(2, vec[0]);
        EXPECT_EQ(2, vec[1]);
    }
    EXPECT_EQ(FooCounted::ctors, FooCounted::dtors);
}

TEST(Yato_Vector1D, insert_empty_usertype_2)
{
    FooCounted::reset_counters();
    {
        yato::vector_1d<FooCounted> vec = { };
        vec.reserve(3);

        yato::vector_1d<FooCounted> line = { 2, 2 };
        vec.insert(vec.begin(), std::begin(line), std::end(line));

        EXPECT_EQ(2u, vec.size(0));
        EXPECT_EQ(2, vec[0]);
        EXPECT_EQ(2, vec[1]);
    }
    EXPECT_EQ(FooCounted::ctors, FooCounted::dtors);
}

TEST(Yato_Vector1D, erase)
{
    yato::vector_1d<int> vec1 = { 1, 4, 5, 2, 3 };

    auto it = vec1.erase(vec1.begin() + 1, vec1.begin() + 3);
    EXPECT_EQ(3u, vec1.size(0));
    EXPECT_EQ(2, (*it));

    it = vec1.erase(it - 1);
    EXPECT_EQ(2u, vec1.size(0));
    EXPECT_EQ(2, (*it));
}

TEST(Yato_Vector1D, erase_usertype)
{
    FooCounted::reset_counters();
    {
        yato::vector_1d<FooCounted> vec1 = { 1, 4, 5, 2, 3 };

        auto it = vec1.erase(vec1.begin() + 1, vec1.begin() + 3);
        EXPECT_EQ(3u, vec1.size(0));
        EXPECT_EQ(2, (*it));

        it = vec1.erase(it - 1);
        EXPECT_EQ(2u, vec1.size(0));
        EXPECT_EQ(2, (*it));
    }
    EXPECT_EQ(FooCounted::ctors, FooCounted::dtors);
}

TEST(Yato_Vector1D, erase_2)
{
    yato::vector_1d<int> vec1 = { 1, 4, 5, 6, 3 };

    auto it = vec1.erase(vec1.begin() + 1, vec1.begin() + 4);
    EXPECT_EQ(2U, vec1.size(0));
    EXPECT_EQ(3, (*it));
}

TEST(Yato_Vector1D, erase_2_usertype)
{
    FooCounted::reset_counters();
    {
        yato::vector_1d<FooCounted> vec1 = { 1, 4, 5, 6, 3 };

        auto it = vec1.erase(vec1.begin() + 1, vec1.begin() + 4);
        EXPECT_EQ(2U, vec1.size(0));
        EXPECT_EQ(3, (*it));
    }
    EXPECT_EQ(FooCounted::ctors, FooCounted::dtors);
}

TEST(Yato_Vector1D, erase_3)
{
    yato::vector_1d<int> vec1 = { 1, 4, 5, 6, 3 };

    vec1.erase(vec1.begin(), vec1.end());
    EXPECT_EQ(0U, vec1.size(0));
}

TEST(Yato_Vector1D, erase_3_usertype)
{
    FooCounted::reset_counters();
    {
        yato::vector_1d<FooCounted> vec1 = { 1, 4, 5, 6, 3 };

        vec1.erase(vec1.begin(), vec1.end());
        EXPECT_EQ(0U, vec1.size(0));
    }
    EXPECT_EQ(FooCounted::ctors, FooCounted::dtors);
}


TEST(Yato_Vector1D, resize)
{
    yato::vector_1d<int> vec1 = { 1, 2, 3, 4 };
    EXPECT_EQ(4u, vec1.size(0));

    vec1.resize(7);
    EXPECT_EQ(7u, vec1.size(0));

    EXPECT_EQ(1, vec1[0]);
    EXPECT_EQ(2, vec1[1]);
    EXPECT_EQ(3, vec1[2]);
    EXPECT_EQ(4, vec1[3]);

    vec1.resize(yato::dims(4));
    EXPECT_EQ(4u, vec1.size(0));

    EXPECT_EQ(1, vec1[0]);
    EXPECT_EQ(2, vec1[1]);
    EXPECT_EQ(3, vec1[2]);
    EXPECT_EQ(4, vec1[3]);

    vec1.resize(yato::dims(1));
    EXPECT_EQ(1u, vec1.size(0));

    EXPECT_EQ(1, vec1[0]);

    vec1.resize(0);
    EXPECT_EQ(0u, vec1.size(0));

    vec1.resize(yato::dims(3), 7);
    EXPECT_EQ(7, vec1[0]);
    EXPECT_EQ(7, vec1[1]);
    EXPECT_EQ(7, vec1[2]);
}

TEST(Yato_Vector1D, resize_usertype)
{
    FooCounted::reset_counters();
    {
        yato::vector_1d<FooCounted> vec1 = { 1, 2, 3, 4 };
        EXPECT_EQ(4u, vec1.size(0));

        vec1.resize(7);
        EXPECT_EQ(7u, vec1.size(0));

        EXPECT_EQ(1, vec1[0]);
        EXPECT_EQ(2, vec1[1]);
        EXPECT_EQ(3, vec1[2]);
        EXPECT_EQ(4, vec1[3]);

        vec1.resize(yato::dims(4));
        EXPECT_EQ(4u, vec1.size(0));

        EXPECT_EQ(1, vec1[0]);
        EXPECT_EQ(2, vec1[1]);
        EXPECT_EQ(3, vec1[2]);
        EXPECT_EQ(4, vec1[3]);

        vec1.resize(yato::dims(1));
        EXPECT_EQ(1u, vec1.size(0));

        EXPECT_EQ(1, vec1[0]);

        vec1.resize(0);
        EXPECT_EQ(0u, vec1.size(0));

        vec1.resize(yato::dims(3), 7);
        EXPECT_EQ(7, vec1[0]);
        EXPECT_EQ(7, vec1[1]);
        EXPECT_EQ(7, vec1[2]);
    }
    EXPECT_EQ(FooCounted::ctors, FooCounted::dtors);
}


namespace
{
    class TestError 
        : public std::runtime_error
    {
    public:
        TestError()
            : std::runtime_error("TestError")
        { }
    };

    class FooThrowing
    {
        int m_val = 0;

        static size_t throw_delay;
    public:
        static size_t ctors;
        static size_t dtors;

        static void reset_counters(size_t throw_after)
        {
            ctors = 0;
            dtors = 0;
            throw_delay = throw_after;
        }

        FooThrowing()
            : m_val(0)
        {
            ++ctors;
        }

        FooThrowing(int v)
            : m_val(v)
        {
            ++ctors;
        }

        FooThrowing(const FooThrowing & other)
            : m_val(other.m_val)
        {
            if(0 == throw_delay) {
                throw TestError{};
            }
            --throw_delay;
            ++ctors;
        }

        FooThrowing(FooThrowing && other) noexcept(false)
            : m_val(other.m_val)
        {
            ++ctors;
        }

        ~FooThrowing()
        {
            ++dtors;
        }

        FooThrowing& operator= (const FooThrowing &) = delete;
        FooThrowing& operator= (FooThrowing &&) = delete;

        operator int() const
        {
            return m_val;
        }
    };

    size_t FooThrowing::throw_delay = 0;
    size_t FooThrowing::ctors = 0;
    size_t FooThrowing::dtors = 0;
}

TEST(Yato_Vector1D, exception_safe_constructor)
{
    bool thrown = false;
    FooThrowing::reset_counters(3);
    try {
         yato::vector_1d<FooThrowing> v(yato::dims(8), FooThrowing(1));
    }
    catch(TestError &) {
        // expected exception
        thrown = true;
    }
    catch(...) {
        // error
        throw;
    }
    EXPECT_TRUE(thrown);
    EXPECT_EQ(FooThrowing::ctors, FooThrowing::dtors);
}

TEST(Yato_Vector1D, exception_safe_constructor_2)
{
    bool thrown = false;
    FooThrowing::reset_counters(3);
    try {
        std::array<FooThrowing, 8> arr {{ 1, 2, 3, 4, 5, 6, 7, 8 }};
        yato::vector_1d<FooThrowing> v(yato::dims(8), arr.cbegin(), arr.cend());
    }
    catch(TestError &) {
        // expected exception
        thrown = true;
    }
    catch(...) {
        // error
        throw;
    }
    EXPECT_TRUE(thrown);
    EXPECT_EQ(FooThrowing::ctors, FooThrowing::dtors);
}

TEST(Yato_Vector1D, exception_safe_constructor_from_ilist)
{
    bool thrown = false;
    FooThrowing::reset_counters(5);
    try {
        yato::vector_1d<FooThrowing> vec4 = { 1, 2, 3, 4, 5, 6 };
    }
    catch(TestError &) {
        // expected exception
        thrown = true;
    }
    catch(...) {
        // error
        throw;
    }
    EXPECT_TRUE(thrown);
    EXPECT_EQ(FooThrowing::ctors, FooThrowing::dtors);
}

TEST(Yato_Vector1D, exception_safe_assign)
{
    bool thrown = false;
    FooThrowing::reset_counters(10);
    try {
        yato::vector_1d<FooThrowing> v(yato::dims(8), FooThrowing(1));
        v.assign(yato::dims(6), 10);
    }
    catch(TestError &) {
        // expected exception
        thrown = true;
    }
    catch(...) {
        // error
        throw;
    }
    EXPECT_TRUE(thrown);
    EXPECT_EQ(FooThrowing::ctors, FooThrowing::dtors);
}

TEST(Yato_Vector1D, exception_safe_assign_2)
{
    bool thrown = false;
    FooThrowing::reset_counters(10);
    try {
        yato::vector_1d<FooThrowing> v(yato::dims(8), FooThrowing(1));
        v.assign(yato::dims(18), 10);
    }
    catch(TestError &) {
        // expected exception
        thrown = true;
    }
    catch(...) {
        // error
        throw;
    }
    EXPECT_TRUE(thrown);
    EXPECT_EQ(FooThrowing::ctors, FooThrowing::dtors);
}


TEST(Yato_Vector1D, exception_safe_reserve)
{
    bool thrown = false;
    FooThrowing::reset_counters(12);
    try {
        yato::vector_1d<FooThrowing> v(yato::dims(8), FooThrowing(1));
        v.reserve(16);
    }
    catch(TestError &) {
        // expected exception
        thrown = true;
    }
    catch(...) {
        // error
        throw;
    }
    EXPECT_TRUE(thrown);
    EXPECT_EQ(FooThrowing::ctors, FooThrowing::dtors);
}

TEST(Yato_Vector1D, exception_safe_shrink_to_fit)
{
    bool thrown = false;
    FooThrowing::reset_counters(20);
    try {
        yato::vector_1d<FooThrowing> v(yato::dims(8), FooThrowing(1));
        v.reserve(16);
        v.shrink_to_fit();
    }
    catch(TestError &) {
        // expected exception
        thrown = true;
    }
    catch(...) {
        // error
        throw;
    }
    EXPECT_TRUE(thrown);
    EXPECT_EQ(FooThrowing::ctors, FooThrowing::dtors);
}


TEST(Yato_Vector1D, exception_safe_resize)
{
    bool thrown = false;
    FooThrowing::reset_counters(12);
    try {
        yato::vector_1d<FooThrowing> v(yato::dims(8), FooThrowing(1));
        v.resize(20, 1);
    }
    catch(TestError &) {
        // expected exception
        thrown = true;
    }
    catch(...) {
        // error
        throw;
    }
    EXPECT_TRUE(thrown);
    EXPECT_EQ(FooThrowing::ctors, FooThrowing::dtors);
}

TEST(Yato_Vector1D, exception_safe_resize_2)
{
    bool thrown = false;
    FooThrowing::reset_counters(12);
    try {
        yato::vector_1d<FooThrowing> v;
        v.resize(yato::dims(27), 1);
    }
    catch(TestError &) {
        // expected exception
        thrown = true;
    }
    catch(...) {
        // error
        throw;
    }
    EXPECT_TRUE(thrown);
    EXPECT_EQ(FooThrowing::ctors, FooThrowing::dtors);
}

TEST(Yato_Vector1D, exception_safe_resize_3)
{
    bool thrown = false;
    FooThrowing::reset_counters(12);
    try {
        yato::vector_1d<FooThrowing> v;
        v.reserve(27);
        v.resize(yato::dims(27), 1);
    }
    catch(TestError &) {
        // expected exception
        thrown = true;
    }
    catch(...) {
        // error
        throw;
    }
    EXPECT_TRUE(thrown);
    EXPECT_EQ(FooThrowing::ctors, FooThrowing::dtors);
}

TEST(Yato_Vector1D, exception_safe_push)
{
    FooThrowing::reset_counters(7);
    bool thrown = false;
    {
        yato::vector_1d<FooThrowing> v = { 1, 2, 3, 4 };
        try {
            FooThrowing x{7};
            v.push_back(x);
        }
        catch(TestError &) {
            // expected exception
            thrown = true;
        }
        catch(...) {
            // error
            throw;
        }
        EXPECT_EQ(4u, v.size(0));
        EXPECT_EQ(1, v[0]);
        EXPECT_EQ(2, v[1]);
        EXPECT_EQ(3, v[2]);
        EXPECT_EQ(4, v[3]);
    }
    EXPECT_TRUE(thrown);
    EXPECT_EQ(FooThrowing::ctors, FooThrowing::dtors);
}

TEST(Yato_Vector1D, exception_safe_push_2)
{
    FooThrowing::reset_counters(8);
    bool thrown = false;
    {
        yato::vector_1d<FooThrowing> v = { 1, 2, 3, 4 };
        v.reserve(5);
        try {
            FooThrowing x{7};
            v.push_back(x);
        }
        catch(TestError &) {
            // expected exception
            thrown = true;
        }
        catch(...) {
            // error
            throw;
        }
        EXPECT_EQ(4u, v.size(0));
        EXPECT_EQ(1, v[0]);
        EXPECT_EQ(2, v[1]);
        EXPECT_EQ(3, v[2]);
        EXPECT_EQ(4, v[3]);
    }
    EXPECT_TRUE(thrown);
    EXPECT_EQ(FooThrowing::ctors, FooThrowing::dtors);
}


TEST(Yato_Vector1D, exception_safe_insert)
{
    FooThrowing::reset_counters(4);
    bool thrown = false;
    {
        yato::vector_1d<FooThrowing> vec  = { 1, 1, 3, 3 };
        try {
            FooThrowing x{2};
            vec.insert(std::next(vec.begin(), 2), x);
        }
        catch(TestError &) {
            // expected exception
            thrown = true;
        }
        catch(...) {
            // error
            throw;
        }
    }
    EXPECT_TRUE(thrown);
    EXPECT_EQ(FooThrowing::ctors, FooThrowing::dtors);
}

TEST(Yato_Vector1D, exception_safe_insert_2)
{
    FooThrowing::reset_counters(8);
    bool thrown = false;
    {
        yato::vector_1d<FooThrowing> vec  = { 1, 1, 3, 3 };
        vec.reserve(5);
        try {
            FooThrowing x{7};
            vec.insert(std::next(vec.begin(), 2), x);
        }
        catch(TestError &) {
            // expected exception
            thrown = true;
        }
        catch(...) {
            // error
            throw;
        }
    }
    EXPECT_TRUE(thrown);
    EXPECT_EQ(FooThrowing::ctors, FooThrowing::dtors);
}

TEST(Yato_Vector1D, exception_safe_insert_3)
{
    FooThrowing::reset_counters(8);
    bool thrown = false;
    {
        yato::vector_1d<FooThrowing> vec  = { 1, 1, 3, 3 };
        vec.reserve(5);
        try {
            FooThrowing x{7};
            vec.insert(std::next(vec.begin(), 2), x);
        }
        catch(TestError &) {
            // expected exception
            thrown = true;
        }
        catch(...) {
            // error
            throw;
        }
    }
    EXPECT_TRUE(thrown);
    EXPECT_EQ(FooThrowing::ctors, FooThrowing::dtors);
}

TEST(Yato_Vector1D, exception_safe_insert_4)
{
    FooThrowing::reset_counters(12);
    bool thrown = false;
    {
        yato::vector_1d<FooThrowing> vec1 = { 1, 2, 3, 4, 5 };
        yato::vector_1d<FooThrowing> vec2 = { 10, 11, 12 };
        try {
            vec1.insert(std::next(vec1.begin(), 3), vec2.begin(), vec2.end());
        }
        catch(TestError &) {
            // expected exception
            thrown = true;
        }
        catch(...) {
            // error
            throw;
        }
    }
    EXPECT_TRUE(thrown);
    EXPECT_EQ(FooThrowing::ctors, FooThrowing::dtors);
}

TEST(Yato_Vector1D, exception_safe_erase)
{
    FooThrowing::reset_counters(6);
    bool thrown = false;
    {
        yato::vector_1d<FooThrowing> vec1 = { 1, 2, 3, 4, 5 };
        try {
            vec1.erase(std::next(vec1.begin(), 1), std::next(vec1.begin(), 3));
        }
        catch(TestError &) {
            // expected exception
            thrown = true;
        }
        catch(...) {
            // error
            throw;
        }
    }
    EXPECT_TRUE(thrown);
    EXPECT_EQ(FooThrowing::ctors, FooThrowing::dtors);
}

TEST(Yato_Vector1D, exception_safe_erase_2)
{
    FooThrowing::reset_counters(9);
    bool thrown = false;
    {
        yato::vector_1d<FooThrowing> vec1 = { 1, 2, 3, 4, 5, 6, 7 };
        try {
            vec1.erase(std::next(vec1.begin(), 2));
        }
        catch(TestError &) {
            // expected exception
            thrown = true;
        }
        catch(...) {
            // error
            throw;
        }
    }
    EXPECT_TRUE(thrown);
    EXPECT_EQ(FooThrowing::ctors, FooThrowing::dtors);
}


