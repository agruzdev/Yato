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

TEST(Yato_VectorND, common)
{
    class A {
        int m_val;
    public:
        A(int x): m_val(x) {};

        int get() const { return m_val; }
    };

    try {
        yato::vector_nd<int, 3> vec0{};
        EXPECT_TRUE(vec0.empty());
        EXPECT_EQ(3U, vec0.dimensions_num());
        EXPECT_EQ(0U, vec0.size(0));
        EXPECT_EQ(0U, vec0.size(1));
        EXPECT_EQ(0U, vec0.size(2));
        EXPECT_EQ(0U, vec0.total_size());

        yato::vector_nd<int, 2> vec1(yato::dims(2, 3));
        EXPECT_FALSE(vec1.empty());
        EXPECT_EQ(2U, vec1.dimensions_num());
        EXPECT_EQ(2U, vec1.size(0));
        EXPECT_EQ(3U, vec1.size(1));
        EXPECT_EQ(6U, vec1.total_size());

        EXPECT_EQ(2U, yato::height(vec1));
        EXPECT_EQ(3U, yato::width(vec1));

        yato::vector_nd<int, 1> vec2(yato::dims(5), 1);
        EXPECT_EQ(5U, vec2.size(0));

        yato::vector_nd<float, 3> vec3(yato::dims(2, 3, 3), 11.0f);

        yato::vector_nd<int, 2> vec4 = { {1, 1, 1}, {2, 2, 2} };
        yato::vector_nd<int, 3> vec5 = { { {1, 1}, {1, 2}, {1, 3} }, { {2, 4}, {2, 5}, {2, 6} } };
        yato::vector_nd<int, 2> vec6 = { };

        EXPECT_EQ(2U, yato::depth(vec5));
        EXPECT_EQ(3U, yato::height(vec5));
        EXPECT_EQ(2U, yato::width(vec5));

        yato::vector_nd<A, 2> vec7 = { { A(1) } };

        std::vector<int> sizes;
        sizes.push_back(1);
        sizes.push_back(5);
        sizes.push_back(2);
        const yato::vector_nd<int, 3> vec8(yato::make_range(sizes.cbegin(), sizes.cend()), 1);

        yato::vector_nd<float, 3> vec9(yato::dims(2, 0, 3), 1.0f); 
        //EXPECT_TRUE(vec9.empty()); Undefined behaviour

        yato::vector_nd<int, 1> vec2c = yato::vector_nd<int, 1>(vec2.view());
        for(size_t i = 0; i < vec2.size(); ++i) {
            EXPECT_EQ(vec2[i], vec2c[i]);
        }
        yato::vector_nd<int, 2> vec4c = yato::vector_nd<int, 2>(vec4.view());
        for(size_t j = 0; j < vec4.size(0); ++j) {
            for(size_t i = 0; i < vec4.size(1); ++i) {
                EXPECT_EQ(vec4[j][i], vec4c[j][i]);
            }
        }
        yato::vector_nd<int, 3> vec5c = yato::vector_nd<int, 3>(vec5.cview());
        for(size_t k = 0; k < vec5.size(0); ++k) {
            for(size_t j = 0; j < vec5.size(1); ++j) {
                for(size_t i = 0; i < vec5.size(2); ++i) {
                    EXPECT_EQ(vec5[k][j][i], vec5c[k][j][i]);
                }
            }
        }
    }
    catch (...)
    {
        EXPECT_TRUE(false);
    }
    EXPECT_TRUE(true);
}

TEST(Yato_VectorND, common_usertype)
{
    FooCounted::reset_counters();
    try {
        yato::vector_nd<FooCounted, 3> vec0{};
        EXPECT_TRUE(vec0.empty());
        EXPECT_EQ(3U, vec0.dimensions_num());
        EXPECT_EQ(0U, vec0.size(0));
        EXPECT_EQ(0U, vec0.size(1));
        EXPECT_EQ(0U, vec0.size(2));
        EXPECT_EQ(0U, vec0.total_size());

        yato::vector_nd<FooCounted, 2> vec1(yato::dims(2, 3));
        EXPECT_FALSE(vec1.empty());
        EXPECT_EQ(2U, vec1.dimensions_num());
        EXPECT_EQ(2U, vec1.size(0));
        EXPECT_EQ(3U, vec1.size(1));
        EXPECT_EQ(6U, vec1.total_size());

        EXPECT_EQ(2U, yato::height(vec1));
        EXPECT_EQ(3U, yato::width(vec1));

        yato::vector_nd<FooCounted, 1> vec2(yato::dims(5), 1);
        EXPECT_EQ(5U, vec2.size(0));

        yato::vector_nd<int, 2> vec4 = { {1, 1, 1}, {2, 2, 2} };
        yato::vector_nd<int, 3> vec5 = { { {1, 1}, {1, 2}, {1, 3} }, { {2, 4}, {2, 5}, {2, 6} } };
        yato::vector_nd<int, 2> vec6 = { };

        EXPECT_EQ(2U, yato::depth(vec5));
        EXPECT_EQ(3U, yato::height(vec5));
        EXPECT_EQ(2U, yato::width(vec5));

        yato::vector_nd<FooCounted, 2> vec7 = { { FooCounted(1) } };

        std::vector<int> sizes;
        sizes.push_back(1);
        sizes.push_back(5);
        sizes.push_back(2);
        const yato::vector_nd<FooCounted, 3> vec8(yato::make_range(sizes.cbegin(), sizes.cend()), 1);

        yato::vector_nd<FooCounted, 3> vec9(yato::dims(2, 0, 3), 42); 
    }
    catch (...)
    {
        EXPECT_TRUE(false);
    }
    EXPECT_EQ(FooCounted::ctors, FooCounted::dtors);
}

TEST(Yato_VectorND, common_2)
{
    yato::vector_nd<int, 2> vec1 = { {1, 1, 1}, {2, 2, 2} };
    yato::vector_nd<int, 2> vec2(vec1);

    EXPECT_EQ(yato::width(vec1), yato::width(vec2));
    EXPECT_EQ(yato::height(vec1), yato::height(vec2));

    yato::vector_nd<int, 2> vec3(std::move(vec1));
    EXPECT_EQ(yato::width(vec3), yato::width(vec2));
    EXPECT_EQ(yato::height(vec3), yato::height(vec2));
}

TEST(Yato_VectorND, ctor_from_range)
{
    int raw[] = { 1, 2, 3, 4, 5, 6 };

    auto v1 = yato::vector_1d<int>(yato::dims(6), std::begin(raw), std::end(raw));
    EXPECT_EQ(6U, v1.size(0));
    EXPECT_EQ(1, v1[0]);
    EXPECT_EQ(2, v1[1]);
    EXPECT_EQ(3, v1[2]);
    EXPECT_EQ(4, v1[3]);
    EXPECT_EQ(5, v1[4]);
    EXPECT_EQ(6, v1[5]);

    auto v2 = yato::vector_2d<int>(yato::dims(2, 3), std::begin(raw), std::end(raw));
    EXPECT_EQ(2U, v2.size(0));
    EXPECT_EQ(3U, v2.size(1));
    EXPECT_EQ(1, v2[0][0]);
    EXPECT_EQ(2, v2[0][1]);
    EXPECT_EQ(3, v2[0][2]);
    EXPECT_EQ(4, v2[1][0]);
    EXPECT_EQ(5, v2[1][1]);
    EXPECT_EQ(6, v2[1][2]);

    auto v3 = yato::vector_3d<int>(yato::dims(1, 1, 6), yato::make_range(std::begin(raw), std::end(raw)));
    EXPECT_EQ(1U, v3.size(0));
    EXPECT_EQ(1U, v3.size(1));
    EXPECT_EQ(6U, v3.size(2));
    EXPECT_EQ(1, v3[0][0][0]);
    EXPECT_EQ(2, v3[0][0][1]);
    EXPECT_EQ(3, v3[0][0][2]);
    EXPECT_EQ(4, v3[0][0][3]);
    EXPECT_EQ(5, v3[0][0][4]);
    EXPECT_EQ(6, v3[0][0][5]);
}

TEST(Yato_VectorND, construct_from_initializer_list)
{
    FooCounted::reset_counters();
    {
        yato::vector_nd<FooCounted, 2> vec0 = {};
        EXPECT_EQ(0u, vec0.total_size());

        yato::vector_nd<FooCounted, 2> vec00 = { {}, {} };
        EXPECT_EQ(0u, vec00.total_size());

        yato::vector_nd<FooCounted, 2> vec1 = { {1, 1, 1}, {2, 2, 2} };
        EXPECT_EQ(6u, vec1.total_size());
        EXPECT_EQ(1, vec1[0][0]);
        EXPECT_EQ(1, vec1[0][1]);
        EXPECT_EQ(1, vec1[0][2]);
        EXPECT_EQ(2, vec1[1][0]);
        EXPECT_EQ(2, vec1[1][1]);
        EXPECT_EQ(2, vec1[1][2]);

        yato::vector_nd<FooCounted, 2> vec2 = { {1, 1, 1}, {2, 4}, {3, 3} };
        EXPECT_EQ(9u, vec2.total_size());
        EXPECT_EQ(1, vec2[0][0]);
        EXPECT_EQ(1, vec2[0][1]);
        EXPECT_EQ(1, vec2[0][2]);
        EXPECT_EQ(2, vec2[1][0]);
        EXPECT_EQ(4, vec2[1][1]);
        EXPECT_EQ(4, vec2[1][2]);
        EXPECT_EQ(3, vec2[2][0]);
        EXPECT_EQ(3, vec2[2][1]);
        EXPECT_EQ(3, vec2[2][2]);

        yato::vector_nd<FooCounted, 2> vec3 = { {1}, {2, 2, 2}, { 3, 4 } };
        EXPECT_EQ(9u, vec3.total_size());
        EXPECT_EQ(1, vec3[0][0]);
        EXPECT_EQ(1, vec3[0][1]);
        EXPECT_EQ(1, vec3[0][2]);
        EXPECT_EQ(2, vec3[1][0]);
        EXPECT_EQ(2, vec3[1][1]);
        EXPECT_EQ(2, vec3[1][2]);
        EXPECT_EQ(3, vec3[2][0]);
        EXPECT_EQ(4, vec3[2][1]);
        EXPECT_EQ(4, vec3[2][2]);

        yato::vector_nd<FooCounted, 3> vec4 = { { {1}, {2, 2, 2}, { 3, 4 } }, { { 10 }, { 20 }, { 30 } } };
        EXPECT_EQ(18u, vec4.total_size());
        EXPECT_EQ(1, vec4[0][0][0]);
        EXPECT_EQ(1, vec4[0][0][1]);
        EXPECT_EQ(1, vec4[0][0][2]);
        EXPECT_EQ(2, vec4[0][1][0]);
        EXPECT_EQ(2, vec4[0][1][1]);
        EXPECT_EQ(2, vec4[0][1][2]);
        EXPECT_EQ(3, vec4[0][2][0]);
        EXPECT_EQ(4, vec4[0][2][1]);
        EXPECT_EQ(4, vec4[0][2][2]);
        EXPECT_EQ(10, vec4[1][0][0]);
        EXPECT_EQ(10, vec4[1][0][1]);
        EXPECT_EQ(10, vec4[1][0][2]);
        EXPECT_EQ(20, vec4[1][1][0]);
        EXPECT_EQ(20, vec4[1][1][1]);
        EXPECT_EQ(20, vec4[1][1][2]);
        EXPECT_EQ(30, vec4[1][2][0]);
        EXPECT_EQ(30, vec4[1][2][1]);
        EXPECT_EQ(30, vec4[1][2][2]);

        yato::vector_nd<FooCounted, 4> vec5 = { { { { 1, 2 }, { 3, 4 } }, { { 5, 6 }, { 7, 8 } } }, { { { 9, 10 }, { 11, 12 } }, { { 13, 14 }, { 15, 16 } } } };
        EXPECT_EQ(16u, vec5.total_size());
        EXPECT_EQ(1,  vec5[0][0][0][0]);
        EXPECT_EQ(2,  vec5[0][0][0][1]);
        EXPECT_EQ(3,  vec5[0][0][1][0]);
        EXPECT_EQ(4,  vec5[0][0][1][1]);
        EXPECT_EQ(5,  vec5[0][1][0][0]);
        EXPECT_EQ(6,  vec5[0][1][0][1]);
        EXPECT_EQ(7,  vec5[0][1][1][0]);
        EXPECT_EQ(8,  vec5[0][1][1][1]);
        EXPECT_EQ(9,  vec5[1][0][0][0]);
        EXPECT_EQ(10, vec5[1][0][0][1]);
        EXPECT_EQ(11, vec5[1][0][1][0]);
        EXPECT_EQ(12, vec5[1][0][1][1]);
        EXPECT_EQ(13, vec5[1][1][0][0]);
        EXPECT_EQ(14, vec5[1][1][0][1]);
        EXPECT_EQ(15, vec5[1][1][1][0]);
        EXPECT_EQ(16, vec5[1][1][1][1]);
    }
    EXPECT_EQ(FooCounted::ctors, FooCounted::dtors);
}

TEST(Yato_VectorND, access)
{
    yato::vector_nd<int, 2> vec1 = { {1, 2}, {3, 4} };
    EXPECT_TRUE(vec1[0][0] == 1);
    EXPECT_TRUE(vec1[0][1] == 2);
    EXPECT_TRUE(vec1[1][0] == 3);
    EXPECT_TRUE(vec1[1][1] == 4);
}

TEST(Yato_VectorND, access_1)
{
    std::array<uint8_t, 3> dims;
    std::generate(dims.begin(), dims.end(), []() { return static_cast<uint8_t>(std::rand() % 101); });

    float val = (std::rand() % 1000) / 10.0f;
    yato::vector_nd<float, 3> vec_nd(yato::make_range(dims.begin(), dims.end()), val);

    for (size_t i = 0; i < dims[0]; ++i) {
        for (size_t j = 0; j < dims[1]; ++j) {
            for (size_t k = 0; k < dims[2]; ++k) {
                float x;
                EXPECT_NO_THROW(x = vec_nd[i][j][k]);
                EXPECT_EQ(val, x);
            }
        }
    }
}

TEST(Yato_VectorND, access_view)
{
    yato::vector_nd<int, 2> vec1 = { {1, 2}, {3, 4} };
    auto view1 = vec1.cview();
    EXPECT_TRUE(view1[0][0] == 1);
    EXPECT_TRUE(view1[0][1] == 2);
    EXPECT_TRUE(view1[1][0] == 3);
    EXPECT_TRUE(view1[1][1] == 4);

    auto view2 = vec1.view();
    view2[0][0] = 11;
    view2[1][0] = 22;
    EXPECT_TRUE(view1[0][0] == 11);
    EXPECT_TRUE(view1[0][1] == 2);
    EXPECT_TRUE(view1[1][0] == 22);
    EXPECT_TRUE(view1[1][1] == 4);
}

TEST(Yato_VectorND, access_view_1)
{
    std::array<uint8_t, 3> dims;
    std::generate(dims.begin(), dims.end(), []() { return static_cast<uint8_t>(std::rand() % 101); });

    float val = (std::rand() % 1000) / 10.0f;
    yato::vector_nd<float, 3> vec_nd(yato::make_range(dims.begin(), dims.end()), val);

    auto view_nd = vec_nd.cview();

    for (size_t i = 0; i < dims[0]; ++i) {
        for (size_t j = 0; j < dims[1]; ++j) {
            for (size_t k = 0; k < dims[2]; ++k) {
                float x;
                EXPECT_NO_THROW(x = view_nd[i][j][k]);
                EXPECT_EQ(val, x);
            }
        }
    }
}

TEST(Yato_VectorND, dims_1)
{
    yato::vector_nd<int, 3> vec5 = { { { 1, 1 },{ 1, 2 },{ 1, 3 } },{ { 2, 4 },{ 2, 5 },{ 2, 6 } } };
    EXPECT_EQ(2U, vec5[0].dimensions_num());
    EXPECT_EQ(3U, vec5[0].size(0));
    EXPECT_EQ(2U, vec5[0].size(1));
    EXPECT_EQ(6U, vec5[0].total_size());

    auto vec0 = yato::vector_nd<int, 3>(yato::dims(2, 0, 2), 1);
    EXPECT_EQ(3U, vec0.dimensions_num());
    EXPECT_EQ(2U, vec0[0].dimensions_num());
    EXPECT_EQ(0U, vec0.total_size());
    EXPECT_EQ(0U, vec0[0].total_size());
}


TEST(Yato_VectorND, operator_at)
{
    yato::vector_nd<int, 2> vec1 = { { 1, 2 },{ 3, 4 } };
    EXPECT_TRUE(vec1.at(0, 0) == 1);
    EXPECT_TRUE(vec1.at(0, 1) == 2);
    EXPECT_TRUE(vec1.at(1, 0) == 3);
    EXPECT_TRUE(vec1.at(1, 1) == 4);
    
    EXPECT_THROW(vec1.at(0, 3), yato::out_of_range_error);
    EXPECT_THROW(vec1.at(3, 1), yato::out_of_range_error);
    EXPECT_THROW(vec1.at(1, 2), yato::out_of_range_error);
    EXPECT_THROW(vec1.at(2, 1), yato::out_of_range_error);
}

TEST(Yato_VectorND, operator_at_1)
{
    std::array<uint8_t, 3> dims;
    std::generate(dims.begin(), dims.end(), []() { return static_cast<uint8_t>(std::rand() % 51); });

    float val = (std::rand() % 1000) / 10.0f;
    yato::vector_nd<float, 3> vec_nd(yato::make_range(dims.begin(), dims.end()), val);

    for (size_t i = 0; i < dims[0]; ++i) {
        for (size_t j = 0; j < dims[1]; ++j) {
            for (size_t k = 0; k < dims[2]; ++k) {
                float x;
                EXPECT_NO_THROW(x = vec_nd.at(i, j, k));
                EXPECT_THROW(vec_nd.at(dims[0] + i, dims[1] + j, dims[2] + k), yato::out_of_range_error);
                EXPECT_EQ(val, x);
            }
        }
    }
}

TEST(Yato_VectorND, reserve)
{
    yato::vector_nd<int, 3> vec;
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

TEST(Yato_VectorND, reserve_usertype)
{
    FooCounted::reset_counters();
    {
        yato::vector_nd<int, 3> vec;
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

TEST(Yato_VectorND, assign)
{
    yato::vector_nd<int, 3> vec = {};
    
    EXPECT_TRUE(vec.empty());
    for (const int & x : vec.plain_crange()) {
        (void)x;
        EXPECT_TRUE(false);
    }

    vec.assign(yato::dims(2, 2, 2), 1);
    
    EXPECT_FALSE(vec.empty());
    for (const int & x : vec.plain_crange()) {
        EXPECT_EQ(1, x);
    }

    vec.assign(yato::dims(1, 1, 1), 42);

    EXPECT_FALSE(vec.empty());
    for (const int & x : vec.plain_crange()) {
        EXPECT_EQ(42, x);
    }

    vec.assign(yato::dims(1, 2, 2), 10);

    EXPECT_FALSE(vec.empty());
    for (const int & x : vec.plain_crange()) {
        EXPECT_EQ(10, x);
    }

}

TEST(Yato_VectorND, assign_usertype)
{
    FooCounted::reset_counters();
    {
        yato::vector_nd<FooCounted, 3> vec = {};
    
        EXPECT_TRUE(vec.empty());
        for (const int & x : vec.plain_crange()) {
            (void)x;
            EXPECT_TRUE(false);
        }

        vec.assign(yato::dims(2, 2, 2), 1);
    
        EXPECT_FALSE(vec.empty());
        for (const int & x : vec.plain_crange()) {
            EXPECT_EQ(1, x);
        }

        vec.assign(yato::dims(1, 1, 1), 42);

        EXPECT_FALSE(vec.empty());
        for (const int & x : vec.plain_crange()) {
            EXPECT_EQ(42, x);
        }

        vec.assign(yato::dims(1, 2, 2), 10);

        EXPECT_FALSE(vec.empty());
        for (const int & x : vec.plain_crange()) {
            EXPECT_EQ(10, x);
        }
    }
    EXPECT_EQ(FooCounted::ctors, FooCounted::dtors);
}

TEST(Yato_VectorND, proxy_iter)
{
    yato::vector_nd<int, 2> vec5 = { { 1, 1 },{ 1, 2 },{ 1, 3 } };

    auto row = vec5.cbegin();
    auto end = vec5.cend();
    for (int i = 1; row != end; ++row, ++i) {
        const int & x = (*row)[0];
        EXPECT_EQ(1, x);
        EXPECT_EQ((*row)[1], i);
    }

    yato::vector_nd<int, 2> vec6 = { { 1, 1, 1 },{ 0, 0, 0 },{ 1, 1, 1 } };
    auto row_2 = std::next(vec6.begin());
    std::fill(std::begin(*row_2), std::end(*row_2), 1);
    for (auto x : vec6.plain_range()) {
        EXPECT_EQ(1, x);
    }
}


TEST(Yato_VectorND, copy_proxy)
{
    yato::vector_nd<int, 3> vec5 = { { { 1, 1 },{ 1, 2 },{ 1, 3 } },{ { 2, 4 },{ 2, 5 },{ 2, 6 } } };

    yato::vector_nd<long, 2> copy1 = {};
    copy1 = vec5[1];
    EXPECT_EQ(2, copy1[0][0]);
    EXPECT_EQ(4, copy1[0][1]);
    EXPECT_EQ(2, copy1[1][0]);
    EXPECT_EQ(5, copy1[1][1]);
    EXPECT_EQ(2, copy1[2][0]);
    EXPECT_EQ(6, copy1[2][1]);

    yato::vector_nd<long long, 1> copy2{ copy1[1] };
    EXPECT_EQ(2, copy2[0]);
    EXPECT_EQ(5, copy2[1]);
}

TEST(Yato_VectorND, push)
{
    const yato::vector_nd<short, 1> vec1d_1 = { 1, 2 };
    const yato::vector_nd<short, 1> vec1d_2 = { 3, 4 };
    const yato::vector_nd<short, 1> vec1d_3 = { 5, 6 };
    const yato::vector_nd<short, 1> vec1d_4 = { 7, 8 };

    yato::vector_nd<int, 2> vec2d_1;
    EXPECT_EQ(0u, vec2d_1.size(0));
    vec2d_1.push_back(vec1d_1);
    EXPECT_EQ(1u, vec2d_1.size(0));
    vec2d_1.push_back(vec1d_2);
    EXPECT_EQ(2u, vec2d_1.size(0));
    vec2d_1.push_back(vec1d_3);
    EXPECT_EQ(3u, vec2d_1.size(0));
    vec2d_1.push_back(vec1d_4);
    EXPECT_EQ(4u, vec2d_1.size(0));

    EXPECT_EQ(1, vec2d_1[0][0]);
    EXPECT_EQ(2, vec2d_1[0][1]);
    EXPECT_EQ(3, vec2d_1[1][0]);
    EXPECT_EQ(4, vec2d_1[1][1]);
    EXPECT_EQ(5, vec2d_1[2][0]);
    EXPECT_EQ(6, vec2d_1[2][1]);
    EXPECT_EQ(7, vec2d_1[3][0]);
    EXPECT_EQ(8, vec2d_1[3][1]);

    yato::vector_nd<int, 2> vec2d_2;
    vec2d_2.reserve(8);
    EXPECT_EQ(0u, vec2d_2.size(0));
    vec2d_2.push_back(vec1d_1);
    EXPECT_EQ(1u, vec2d_2.size(0));
    vec2d_2.push_back(vec1d_2);
    EXPECT_EQ(2u, vec2d_2.size(0));
    vec2d_2.push_back(vec1d_3);
    EXPECT_EQ(3u, vec2d_2.size(0));
    vec2d_2.push_back(vec1d_4);
    EXPECT_EQ(4u, vec2d_2.size(0));

    EXPECT_EQ(1, vec2d_2[0][0]);
    EXPECT_EQ(2, vec2d_2[0][1]);
    EXPECT_EQ(3, vec2d_2[1][0]);
    EXPECT_EQ(4, vec2d_2[1][1]);
    EXPECT_EQ(5, vec2d_2[2][0]);
    EXPECT_EQ(6, vec2d_2[2][1]);
    EXPECT_EQ(7, vec2d_2[3][0]);
    EXPECT_EQ(8, vec2d_2[3][1]);
}

TEST(Yato_VectorND, push_usertype)
{
    FooCounted::reset_counters();
    {
        const yato::vector_nd<short, 1> vec1d_1 = { 1, 2 };
        const yato::vector_nd<short, 1> vec1d_2 = { 3, 4 };
        const yato::vector_nd<short, 1> vec1d_3 = { 5, 6 };
        const yato::vector_nd<short, 1> vec1d_4 = { 7, 8 };

        yato::vector_nd<int, 2> vec2d_1;
        EXPECT_EQ(0u, vec2d_1.size(0));
        vec2d_1.push_back(vec1d_1);
        EXPECT_EQ(1u, vec2d_1.size(0));
        vec2d_1.push_back(vec1d_2);
        EXPECT_EQ(2u, vec2d_1.size(0));
        vec2d_1.push_back(vec1d_3);
        EXPECT_EQ(3u, vec2d_1.size(0));
        vec2d_1.push_back(vec1d_4);
        EXPECT_EQ(4u, vec2d_1.size(0));

        EXPECT_EQ(1, vec2d_1[0][0]);
        EXPECT_EQ(2, vec2d_1[0][1]);
        EXPECT_EQ(3, vec2d_1[1][0]);
        EXPECT_EQ(4, vec2d_1[1][1]);
        EXPECT_EQ(5, vec2d_1[2][0]);
        EXPECT_EQ(6, vec2d_1[2][1]);
        EXPECT_EQ(7, vec2d_1[3][0]);
        EXPECT_EQ(8, vec2d_1[3][1]);

        yato::vector_nd<int, 2> vec2d_2;
        vec2d_2.reserve(8);
        EXPECT_EQ(0u, vec2d_2.size(0));
        vec2d_2.push_back(vec1d_1);
        EXPECT_EQ(1u, vec2d_2.size(0));
        vec2d_2.push_back(vec1d_2);
        EXPECT_EQ(2u, vec2d_2.size(0));
        vec2d_2.push_back(vec1d_3);
        EXPECT_EQ(3u, vec2d_2.size(0));
        vec2d_2.push_back(vec1d_4);
        EXPECT_EQ(4u, vec2d_2.size(0));

        EXPECT_EQ(1, vec2d_2[0][0]);
        EXPECT_EQ(2, vec2d_2[0][1]);
        EXPECT_EQ(3, vec2d_2[1][0]);
        EXPECT_EQ(4, vec2d_2[1][1]);
        EXPECT_EQ(5, vec2d_2[2][0]);
        EXPECT_EQ(6, vec2d_2[2][1]);
        EXPECT_EQ(7, vec2d_2[3][0]);
        EXPECT_EQ(8, vec2d_2[3][1]);
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

TEST(Yato_VectorND, push_2)
{
    yato::vector_nd<FooMoving, 1> vec1d_1 = { 1, 2 };
    yato::vector_nd<FooMoving, 1> vec1d_2 = { 3, 4 };
    yato::vector_nd<FooMoving, 1> vec1d_3 = { 5, 6 };
    yato::vector_nd<FooMoving, 1> vec1d_4 = { 7, 8 };

    yato::vector_nd<FooMoving, 2> vec2d_1;
    EXPECT_EQ(0u, vec2d_1.size(0));
    vec2d_1.push_back(std::move(vec1d_1));
    EXPECT_EQ(1u, vec2d_1.size(0));
    vec2d_1.push_back(std::move(vec1d_2));
    EXPECT_EQ(2u, vec2d_1.size(0));
    vec2d_1.push_back(std::move(vec1d_3));
    EXPECT_EQ(3u, vec2d_1.size(0));
    vec2d_1.push_back(std::move(vec1d_4));
    EXPECT_EQ(4u, vec2d_1.size(0));

    EXPECT_EQ(1, vec2d_1[0][0]);
    EXPECT_EQ(2, vec2d_1[0][1]);
    EXPECT_EQ(3, vec2d_1[1][0]);
    EXPECT_EQ(4, vec2d_1[1][1]);
    EXPECT_EQ(5, vec2d_1[2][0]);
    EXPECT_EQ(6, vec2d_1[2][1]);
    EXPECT_EQ(7, vec2d_1[3][0]);
    EXPECT_EQ(8, vec2d_1[3][1]);

    EXPECT_EQ(-1, vec1d_1[0]);
    EXPECT_EQ(-1, vec1d_1[1]);
    EXPECT_EQ(-1, vec1d_2[0]);
    EXPECT_EQ(-1, vec1d_2[1]);
    EXPECT_EQ(-1, vec1d_3[0]);
    EXPECT_EQ(-1, vec1d_3[1]);
    EXPECT_EQ(-1, vec1d_4[0]);
    EXPECT_EQ(-1, vec1d_4[1]);
}

TEST(Yato_VectorND, push_pop)
{
    yato::vector_nd<short, 1> vec1d_1 = { 1, 2 };
    yato::vector_nd<uint8_t, 1> vec1d_2 = { 3, 4 };

    yato::vector_nd<int, 2>  vec2d = { {1, 2}, {3, 4} };
    vec2d.clear();
    EXPECT_TRUE(vec2d.empty());

    vec2d.push_back(vec1d_1);
    vec2d.push_back(vec1d_2);

    yato::vector_nd<long, 3> vec3d = {};

    vec3d.push_back(vec2d);
    EXPECT_EQ(1U, vec3d.size(0));
    EXPECT_EQ(2U, vec3d.size(1));
    EXPECT_EQ(2U, vec3d.size(2));

    vec3d.push_back(vec2d);
    EXPECT_EQ(2U, vec3d.size(0));
    EXPECT_EQ(2U, vec3d.size(1));
    EXPECT_EQ(2U, vec3d.size(2));

    vec3d.push_back(vec2d);
    EXPECT_EQ(3U, vec3d.size(0));
    EXPECT_EQ(2U, vec3d.size(1));
    EXPECT_EQ(2U, vec3d.size(2));

    vec3d.pop_back();
    EXPECT_EQ(2U, vec3d.size(0));
    EXPECT_EQ(2U, vec3d.size(1));
    EXPECT_EQ(2U, vec3d.size(2));
    
    vec3d.pop_back();
    EXPECT_EQ(1U, vec3d.size(0));
    EXPECT_EQ(2U, vec3d.size(1));
    EXPECT_EQ(2U, vec3d.size(2));

    vec3d.pop_back();
    EXPECT_TRUE(vec3d.empty());
}


TEST(Yato_VectorND, vec_1D)
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

TEST(Yato_VectorND, insert)
{
    yato::vector_nd<int, 2> vec = { { 1, 1 },{ 3, 3 } };

    yato::vector_nd<short, 1> line = { 2, 2 };
    vec.insert(std::next(vec.begin()), line);

    vec.insert(vec.begin(), yato::vector_nd<int, 1>{ 0, 0 } );
    vec.insert(vec.end(), yato::vector_nd<int, 1>{ 4, 4 });

    int i = 0;
    for (const auto & row : vec) {
        EXPECT_EQ(i, row[0]);
        EXPECT_EQ(i, row[1]);
        ++i;
    }
}


TEST(Yato_VectorND, insert_usertype)
{
    FooCounted::reset_counters();
    {
        yato::vector_nd<FooCounted, 2> vec = { { 1, 1 },{ 3, 3 } };

        yato::vector_nd<int, 1> line = { 2, 2 };
        vec.insert(std::next(vec.begin()), line);

        vec.insert(vec.begin(), yato::vector_nd<int, 1>{ 0, 0 } );
        vec.insert(vec.end(), yato::vector_nd<int, 1>{ 4, 4 });

        int i = 0;
        for (const auto & row : vec) {
            EXPECT_EQ(i, row[0]);
            EXPECT_EQ(i, row[1]);
            ++i;
        }
    }
    EXPECT_EQ(FooCounted::ctors, FooCounted::dtors);
}

TEST(Yato_VectorND, insert_usertype_2)
{
    FooCounted::reset_counters();
    {
        yato::vector_nd<FooCounted, 2> vec = { { 1, 1 },{ 3, 3 } };

        yato::vector_nd<FooCounted, 1> line = { 2, 2 };
        vec.insert(std::next(vec.begin()), line);

        vec.insert(vec.begin(), yato::vector_nd<FooCounted, 1>{ 0, 0 } );
        vec.insert(vec.end(), yato::vector_nd<FooCounted, 1>{ 4, 4 });

        int i = 0;
        for (const auto & row : vec) {
            EXPECT_EQ(i, row[0]);
            EXPECT_EQ(i, row[1]);
            ++i;
        }
    }
    EXPECT_EQ(FooCounted::ctors, FooCounted::dtors);
}

TEST(Yato_VectorND, insert_2)
{
    yato::vector_nd<int, 2> vec = { { 1, 1 },{ 3, 3 } };
    vec.reserve(10);

    yato::vector_nd<short, 1> line = { 2, 2 };
    vec.insert(std::next(vec.begin()), line);

    vec.insert(vec.begin(), yato::vector_nd<int, 1>{ 0, 0 } );
    vec.insert(vec.end(), yato::vector_nd<int, 1>{ 4, 4 });

    int i = 0;
    for (const auto & row : vec) {
        EXPECT_EQ(i, row[0]);
        EXPECT_EQ(i, row[1]);
        ++i;
    }
}

TEST(Yato_VectorND, insert_3)
{
    yato::vector_nd<int, 2> vec = { { 1, 1 },{ 3, 3 } };
    vec.reserve(24);

    yato::vector_nd<short, 1> line = { 2, 2 };
    vec.insert(std::next(vec.begin()), 3, line);

    vec.insert(vec.begin(), 5, yato::vector_nd<int, 1>{ 0, 0 } );
    vec.insert(vec.end(), 2, yato::vector_nd<int, 1>{ 4, 4 });

    int i = 0;
    std::array<int, 12> row_values{{ 0, 0, 0, 0, 0, 1, 2, 2, 2, 3, 4, 4 }};
    for (const auto & row : vec) {
        EXPECT_EQ(row_values[i], row[0]);
        EXPECT_EQ(row_values[i], row[1]);
        ++i;
    }
}

TEST(Yato_VectorND, insert_2_usertype)
{
    FooCounted::reset_counters();
    {
        yato::vector_nd<FooCounted, 2> vec = { { 1, 1 },{ 3, 3 } };
        vec.reserve(10);

        yato::vector_nd<FooCounted, 1> line = { 2, 2 };
        vec.insert(std::next(vec.begin()), line);

        vec.insert(vec.begin(), yato::vector_nd<FooCounted, 1>{ 0, 0 } );
        vec.insert(vec.end(), yato::vector_nd<FooCounted, 1>{ 4, 4 });

        int i = 0;
        for (const auto & row : vec) {
            EXPECT_EQ(i, row[0]);
            EXPECT_EQ(i, row[1]);
            ++i;
        }
    }
    EXPECT_EQ(FooCounted::ctors, FooCounted::dtors);
}

TEST(Yato_VectorND, insert_3_usertype)
{
    FooCounted::reset_counters();
    {
        yato::vector_nd<FooCounted, 2> vec = { { 1, 1 },{ 3, 3 } };
        vec.reserve(24);

        yato::vector_nd<FooCounted, 1> line = { 2, 2 };
        vec.insert(std::next(vec.begin()), 3, line);

        vec.insert(vec.begin(), 5, yato::vector_nd<FooCounted, 1>{ 0, 0 } );
        vec.insert(vec.end(), 2, yato::vector_nd<FooCounted, 1>{ 4, 4 });

        int i = 0;
        std::array<int, 12> row_values{{ 0, 0, 0, 0, 0, 1, 2, 2, 2, 3, 4, 4 }};
        for (const auto & row : vec) {
            EXPECT_EQ(row_values[i], row[0]);
            EXPECT_EQ(row_values[i], row[1]);
            ++i;
        }
    }
    EXPECT_EQ(FooCounted::ctors, FooCounted::dtors);
}

TEST(Yato_VectorND, insert_count)
{
    yato::vector_nd<int, 2> vec = { { 1, 1 },{ 3, 3 } };

    yato::vector_nd<short, 1> line = { 2, 2 };
    vec.insert(std::next(vec.begin()), 10, line);

    EXPECT_EQ(12u, vec.size(0));

    int i = 0;
    for (const auto & row : vec) {
        if (i == 0) {
                EXPECT_EQ(1, row[0]);
                EXPECT_EQ(1, row[1]);
            }
            if (i > 0 && i < 11) {
                EXPECT_EQ(2, row[0]);
                EXPECT_EQ(2, row[1]);
            }
            if (i == 11) {
                EXPECT_EQ(3, row[0]);
                EXPECT_EQ(3, row[1]);
            }
        ++i;
    }
}


TEST(Yato_VectorND, insert_count_usertype)
{
    FooCounted::reset_counters();
    {
        yato::vector_nd<FooCounted, 2> vec = { { 1, 1 },{ 3, 3 } };

        yato::vector_nd<FooCounted, 1> line = { 2, 2 };
        vec.insert(std::next(vec.begin()), 10, line);

        EXPECT_EQ(12u, vec.size(0));

        int i = 0;
        for (const auto & row : vec) {
            if (i == 0) {
                EXPECT_EQ(1, row[0]);
                EXPECT_EQ(1, row[1]);
            }
            if (i > 0 && i < 11) {
                EXPECT_EQ(2, row[0]);
                EXPECT_EQ(2, row[1]);
            }
            if (i == 11) {
                EXPECT_EQ(3, row[0]);
                EXPECT_EQ(3, row[1]);
            }
            ++i;
        }
    }
    EXPECT_EQ(FooCounted::ctors, FooCounted::dtors);
}


TEST(Yato_VectorND, insert_range)
{
    yato::vector_nd<int, 2> vec1 = { { 1, 1 },{ 4, 4 } };
    yato::vector_nd<int, 2> vec2 = { { 2, 2 },{ 3, 3 } };

    vec1.insert(std::next(vec1.begin()), vec2.begin(), vec2.end());

    int i = 1;
    for (const auto & row : vec1) {
        EXPECT_EQ(i, row[0]);
        EXPECT_EQ(i, row[1]);
        ++i;
    }
}

TEST(Yato_VectorND, insert_range_non_continuous)
{
    yato::vector_nd<int, 2> vec1 = { { 1, 1 },{ 4, 4 } };
    yato::vector_nd<int, 2> vec2 = { { 2, 2, 5 },{ 3, 3, 5 } };

    yato::array_view_2d<int> view2(&vec2[0][0], yato::dims(2, 2), yato::dims(3));

    vec1.insert(std::next(vec1.begin()), view2.begin(), view2.end());

    int i = 1;
    for (const auto & row : vec1) {
        EXPECT_EQ(i, row[0]);
        EXPECT_EQ(i, row[1]);
        ++i;
    }
}


TEST(Yato_VectorND, insert_range_usertype)
{
    FooCounted::reset_counters();
    {
        yato::vector_nd<FooCounted, 2> vec1 = { { 1, 1 },{ 4, 4 } };
        yato::vector_nd<FooCounted, 2> vec2 = { { 2, 2 },{ 3, 3 } };

        vec1.insert(std::next(vec1.begin()), vec2.begin(), vec2.end());

        int i = 1;
        for (const auto & row : vec1) {
            EXPECT_EQ(i, row[0]);
            EXPECT_EQ(i, row[1]);
            ++i;
        }
    }
    EXPECT_EQ(FooCounted::ctors, FooCounted::dtors);
}

TEST(Yato_VectorND, insert_move)
{
    yato::vector_nd<FooMoving, 2> vec1 = { { 1, 1 },{ 4, 4 } };
    yato::vector_nd<FooMoving, 2> vec2 = { { 2, 2 },{ 3, 3 } };

    vec1.insert(std::next(vec1.begin()), yato::make_move_iterator(vec2.begin()), yato::make_move_iterator(vec2.end()));

    int i = 1;
    for (const auto & row : vec1) {
        EXPECT_EQ(i, row[0]);
        EXPECT_EQ(i, row[1]);
        ++i;
    }
    for (const auto & row : vec2) {
        EXPECT_EQ(-1, row[0]);
        EXPECT_EQ(-1, row[1]);
    }
}

TEST(Yato_VectorND, insert_empty)
{
    yato::vector_nd<int, 2> vec = { };

    yato::vector_nd<short, 1> line = { 2, 2 };
    vec.insert(vec.begin(), line);

    EXPECT_EQ(1u, vec.size(0));
    EXPECT_EQ(2, vec[0][0]);
    EXPECT_EQ(2, vec[0][1]);
}

TEST(Yato_VectorND, insert_empty_2)
{
    yato::vector_nd<int, 2> vec = { };
    vec.reserve(2);

    yato::vector_nd<short, 1> line = { 2, 2 };
    vec.insert(vec.begin(), line);

    EXPECT_EQ(1u, vec.size(0));
    EXPECT_EQ(2, vec[0][0]);
    EXPECT_EQ(2, vec[0][1]);
}

TEST(Yato_VectorND, insert_empty_usertype)
{
    FooCounted::reset_counters();
    {
        yato::vector_nd<int, 2> vec = { };

        yato::vector_nd<short, 1> line = { 2, 2 };
        vec.insert(vec.begin(), line);

        EXPECT_EQ(1u, vec.size(0));
        EXPECT_EQ(2, vec[0][0]);
        EXPECT_EQ(2, vec[0][1]);
    }
    EXPECT_EQ(FooCounted::ctors, FooCounted::dtors);
}

TEST(Yato_VectorND, insert_empty_usertype_2)
{
    FooCounted::reset_counters();
    {
        yato::vector_nd<int, 2> vec = { };
        vec.reserve(2);

        yato::vector_nd<short, 1> line = { 2, 2 };
        vec.insert(vec.begin(), line);

        EXPECT_EQ(1u, vec.size(0));
        EXPECT_EQ(2, vec[0][0]);
        EXPECT_EQ(2, vec[0][1]);
    }
    EXPECT_EQ(FooCounted::ctors, FooCounted::dtors);
}

TEST(Yato_VectorND, erase)
{
    yato::vector_nd<int, 2> vec1 = { { 1, 1 },{ 4, 4 }, { 5, 5 }, { 2, 2 }, { 3, 3 } };

    auto it = vec1.erase(vec1.begin() + 1, vec1.begin() + 3);
    EXPECT_EQ(3U, vec1.size(0));
    EXPECT_EQ(2, (*it)[0]);
    EXPECT_EQ(2, (*it)[1]);

    it = vec1.erase(it - 1);
    EXPECT_EQ(2U, vec1.size(0));
    EXPECT_EQ(2, (*it)[0]);
    EXPECT_EQ(2, (*it)[1]);
}

TEST(Yato_VectorND, erase_usertype)
{
    FooCounted::reset_counters();
    {
        yato::vector_nd<FooCounted, 2> vec1 = { { 1, 1 },{ 4, 4 }, { 5, 5 }, { 2, 2 }, { 3, 3 } };

        auto it = vec1.erase(vec1.begin() + 1, vec1.begin() + 3);
        EXPECT_EQ(3U, vec1.size(0));
        EXPECT_EQ(2, (*it)[0]);
        EXPECT_EQ(2, (*it)[1]);
    
        it = vec1.erase(it - 1);
        EXPECT_EQ(2U, vec1.size(0));
        EXPECT_EQ(2, (*it)[0]);
        EXPECT_EQ(2, (*it)[1]);
    }
    EXPECT_EQ(FooCounted::ctors, FooCounted::dtors);
}

TEST(Yato_VectorND, erase_2)
{
    yato::vector_nd<int, 2> vec1 = { { 1, 1 },{ 4, 4 }, { 5, 5 }, { 6, 6 }, { 3, 3 } };

    auto it = vec1.erase(vec1.begin() + 1, vec1.begin() + 4);
    EXPECT_EQ(2U, vec1.size(0));
    EXPECT_EQ(3, (*it)[0]);
    EXPECT_EQ(3, (*it)[1]);
}

TEST(Yato_VectorND, erase_2_usertype)
{
    FooCounted::reset_counters();
    {
        yato::vector_nd<FooCounted, 2> vec1 = { { 1, 1 },{ 4, 4 }, { 5, 5 }, { 2, 2 }, { 3, 3 } };

        auto it = vec1.erase(vec1.begin() + 1, vec1.begin() + 4);
        EXPECT_EQ(2U, vec1.size(0));
        EXPECT_EQ(3, (*it)[0]);
        EXPECT_EQ(3, (*it)[1]);
    }
    EXPECT_EQ(FooCounted::ctors, FooCounted::dtors);
}

TEST(Yato_VectorND, erase_3)
{
    yato::vector_nd<int, 2> vec1 = { { 1, 1 },{ 4, 4 }, { 5, 5 }, { 6, 6 }, { 3, 3 } };

    vec1.erase(vec1.begin(), vec1.end());
    EXPECT_EQ(0U, vec1.size(0));
}

TEST(Yato_VectorND, erase_3_usertype)
{
    FooCounted::reset_counters();
    {
        yato::vector_nd<FooCounted, 2> vec1 = { { 1, 1 },{ 4, 4 }, { 5, 5 }, { 2, 2 }, { 3, 3 } };

        vec1.erase(vec1.begin(), vec1.end());
        EXPECT_EQ(0U, vec1.size(0));
    }
    EXPECT_EQ(FooCounted::ctors, FooCounted::dtors);
}


TEST(Yato_VectorND, resize)
{
    yato::vector_nd<int, 2> vec1 = { { 1, 2 },{ 3, 4 } };
    EXPECT_EQ(2U, vec1.size(0));
    EXPECT_EQ(2U, vec1.size(1));

    vec1.resize(3);
    EXPECT_EQ(3U, vec1.size(0));
    EXPECT_EQ(2U, vec1.size(1));

    EXPECT_EQ(1, vec1[0][0]);
    EXPECT_EQ(2, vec1[0][1]);
    EXPECT_EQ(3, vec1[1][0]);
    EXPECT_EQ(4, vec1[1][1]);

    vec1.resize(yato::dims(4, 3));
    EXPECT_EQ(4U, vec1.size(0));
    EXPECT_EQ(3U, vec1.size(1));

    vec1.resize(yato::dims(1, 1));
    EXPECT_EQ(1U, vec1.size(0));
    EXPECT_EQ(1U, vec1.size(1));

    vec1.resize(0);
    EXPECT_EQ(0U, vec1.size(0));

    vec1.resize(yato::dims(2, 2), 7);
    EXPECT_EQ(7, vec1[0][0]);
    EXPECT_EQ(7, vec1[0][1]);
    EXPECT_EQ(7, vec1[1][0]);
    EXPECT_EQ(7, vec1[1][1]);
}

TEST(Yato_VectorND, resize_usertype)
{
    FooCounted::reset_counters();
    {
        yato::vector_nd<int, 2> vec1 = { { 1, 2 },{ 3, 4 } };
        EXPECT_EQ(2U, vec1.size(0));
        EXPECT_EQ(2U, vec1.size(1));

        vec1.resize(3);
        EXPECT_EQ(3U, vec1.size(0));
        EXPECT_EQ(2U, vec1.size(1));

        EXPECT_EQ(1, vec1[0][0]);
        EXPECT_EQ(2, vec1[0][1]);
        EXPECT_EQ(3, vec1[1][0]);
        EXPECT_EQ(4, vec1[1][1]);

        vec1.resize(yato::dims(4, 3));
        EXPECT_EQ(4U, vec1.size(0));
        EXPECT_EQ(3U, vec1.size(1));

        vec1.resize(yato::dims(1, 1));
        EXPECT_EQ(1U, vec1.size(0));
        EXPECT_EQ(1U, vec1.size(1));

        vec1.resize(0);
        EXPECT_EQ(0U, vec1.size(0));

        vec1.resize(yato::dims(2, 2), 7);
        EXPECT_EQ(7, vec1[0][0]);
        EXPECT_EQ(7, vec1[0][1]);
        EXPECT_EQ(7, vec1[1][0]);
        EXPECT_EQ(7, vec1[1][1]);
    }
    EXPECT_EQ(FooCounted::ctors, FooCounted::dtors);
}

TEST(Yato_VectorND, reshape)
{
    int raw[] = { 1, 2, 3, 4, 5, 6 };

    auto plain_vec = yato::vector_1d<int>(yato::dims(6), std::begin(raw), std::end(raw));
    EXPECT_EQ(6U, plain_vec.size(0));
    EXPECT_EQ(6U, plain_vec.total_size());

    auto vec_2x3 = plain_vec.reshape(yato::dims(2, 3));
    EXPECT_EQ(2U, vec_2x3.size(0));
    EXPECT_EQ(3U, vec_2x3.size(1));
    EXPECT_EQ(6U, vec_2x3.total_size());
    EXPECT_EQ(1, vec_2x3[0][0]);
    EXPECT_EQ(2, vec_2x3[0][1]);
    EXPECT_EQ(3, vec_2x3[0][2]);
    EXPECT_EQ(4, vec_2x3[1][0]);
    EXPECT_EQ(5, vec_2x3[1][1]);
    EXPECT_EQ(6, vec_2x3[1][2]);

    auto vec_3x2 = vec_2x3.reshape(yato::dims(3, 2));
    EXPECT_EQ(3U, vec_3x2.size(0));
    EXPECT_EQ(2U, vec_3x2.size(1));
    EXPECT_EQ(6U, vec_3x2.total_size());
    EXPECT_EQ(1, vec_3x2[0][0]);
    EXPECT_EQ(2, vec_3x2[0][1]);
    EXPECT_EQ(3, vec_3x2[1][0]);
    EXPECT_EQ(4, vec_3x2[1][1]);
    EXPECT_EQ(5, vec_3x2[2][0]);
    EXPECT_EQ(6, vec_3x2[2][1]);

    auto vec_6 = vec_3x2.reshape(yato::dims(6));
    EXPECT_EQ(6U, vec_6.size(0));
    EXPECT_EQ(6U, vec_6.total_size());
    EXPECT_EQ(1, vec_6[0]);
    EXPECT_EQ(2, vec_6[1]);
    EXPECT_EQ(3, vec_6[2]);
    EXPECT_EQ(4, vec_6[3]);
    EXPECT_EQ(5, vec_6[4]);
    EXPECT_EQ(6, vec_6[5]);
}

TEST(Yato_VectorND, reshape_2)
{
    int raw[] = { 1, 2, 3, 4, 5, 6 };

    auto plain_vec = yato::vector_1d<int>(yato::dims(6), std::begin(raw), std::end(raw));
    EXPECT_EQ(6U, plain_vec.size(0));
    EXPECT_EQ(6U, plain_vec.total_size());

    auto vec_2x3 = std::move(plain_vec).reshape(yato::dims(2, 3));
    EXPECT_EQ(2U, vec_2x3.size(0));
    EXPECT_EQ(3U, vec_2x3.size(1));
    EXPECT_EQ(6U, vec_2x3.total_size());
    EXPECT_EQ(1, vec_2x3[0][0]);
    EXPECT_EQ(2, vec_2x3[0][1]);
    EXPECT_EQ(3, vec_2x3[0][2]);
    EXPECT_EQ(4, vec_2x3[1][0]);
    EXPECT_EQ(5, vec_2x3[1][1]);
    EXPECT_EQ(6, vec_2x3[1][2]);

    auto vec_3x2 = std::move(vec_2x3).reshape(yato::dims(3, 2));
    EXPECT_EQ(3U, vec_3x2.size(0));
    EXPECT_EQ(2U, vec_3x2.size(1));
    EXPECT_EQ(6U, vec_3x2.total_size());
    EXPECT_EQ(1, vec_3x2[0][0]);
    EXPECT_EQ(2, vec_3x2[0][1]);
    EXPECT_EQ(3, vec_3x2[1][0]);
    EXPECT_EQ(4, vec_3x2[1][1]);
    EXPECT_EQ(5, vec_3x2[2][0]);
    EXPECT_EQ(6, vec_3x2[2][1]);

    auto vec_6 = std::move(vec_3x2).reshape(yato::dims(6));
    EXPECT_EQ(6U, vec_6.size(0));
    EXPECT_EQ(6U, vec_6.total_size());
    EXPECT_EQ(1, vec_6[0]);
    EXPECT_EQ(2, vec_6[1]);
    EXPECT_EQ(3, vec_6[2]);
    EXPECT_EQ(4, vec_6[3]);
    EXPECT_EQ(5, vec_6[4]);
    EXPECT_EQ(6, vec_6[5]);
}

namespace
{
    int foo(int* p)
    {
        return *p;
    }

    int cfoo(const int* p)
    {
        return *p;
    }
}

TEST(Yato_VectorND, proxy_data)
{
    yato::vector_3d<int> v = { { { 1, 2 }, { 3, 4 } }, { { 5, 6 }, { 7, 8 } } };
    EXPECT_EQ(&v[0][0][0], v.data());
    EXPECT_EQ(&v[0][0][0], v[0].data());
    EXPECT_EQ(&v[0][0][0], v[0][0].data());

    EXPECT_EQ(&v[0][0][0], v.data());
    EXPECT_EQ(&v[1][0][0], v[1].data());
    EXPECT_EQ(&v[1][1][0], v[1][1].data());

    auto plain  = *(v.begin()  + 1);
    auto cplain = *(v.cbegin() + 1);

    EXPECT_EQ(5, foo(plain.data()));
    //EXPECT_EQ(5, foo(cplain.data())); // Error, const iterator
    EXPECT_EQ(5, cfoo(plain.data()));
    EXPECT_EQ(5, cfoo(cplain.data()));
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

TEST(Yato_VectorND, exception_safe_constructor)
{
    bool thrown = false;
    FooThrowing::reset_counters(3);
    try {
         yato::vector_3d<FooThrowing> v(yato::dims(2, 2, 2), FooThrowing(1));
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

TEST(Yato_VectorND, exception_safe_constructor_2)
{
    bool thrown = false;
    FooThrowing::reset_counters(3);
    try {
        std::array<FooThrowing, 8> arr {{ 1, 2, 3, 4, 5, 6, 7, 8 }};
        yato::vector_3d<FooThrowing> v(yato::dims(2, 2, 2), arr.cbegin(), arr.cend());
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

TEST(Yato_VectorND, exception_safe_constructor_from_ilist)
{
    bool thrown = false;
    FooThrowing::reset_counters(10000);
    try {
        yato::vector_nd<FooThrowing, 3> vec4 = { { {1}, {2, 2, 2}, {3} }, { { 10 }, { 30 } } };
    }
    catch(yato::argument_error &) {
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


TEST(Yato_VectorND, exception_safe_constructor_from_ilist_2)
{
    bool thrown = false;
    FooThrowing::reset_counters(10);
    try {
        yato::vector_nd<FooThrowing, 3> vec4 = { { { 1 }, { 2, 2, 2 }, { 3 } }, { { 10 }, { 30 }, { 40 } } };
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

TEST(Yato_VectorND, exception_safe_assign)
{
    bool thrown = false;
    FooThrowing::reset_counters(10);
    try {
        yato::vector_3d<FooThrowing> v(yato::dims(2, 2, 2), FooThrowing(1));
        v.assign(yato::dims(2, 2, 1), 10);
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

TEST(Yato_VectorND, exception_safe_assign_2)
{
    bool thrown = false;
    FooThrowing::reset_counters(10);
    try {
        yato::vector_3d<FooThrowing> v(yato::dims(2, 2, 2), FooThrowing(1));
        v.assign(yato::dims(2, 3, 3), 10);
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


TEST(Yato_VectorND, exception_safe_reserve)
{
    bool thrown = false;
    FooThrowing::reset_counters(12);
    try {
        yato::vector_3d<FooThrowing> v(yato::dims(2, 2, 2), FooThrowing(1));
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

TEST(Yato_VectorND, exception_safe_shrink_to_fit)
{
    bool thrown = false;
    FooThrowing::reset_counters(20);
    try {
        yato::vector_3d<FooThrowing> v(yato::dims(2, 2, 2), FooThrowing(1));
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


TEST(Yato_VectorND, exception_safe_resize)
{
    bool thrown = false;
    FooThrowing::reset_counters(12);
    try {
        yato::vector_3d<FooThrowing> v(yato::dims(2, 2, 2), FooThrowing(1));
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

TEST(Yato_VectorND, exception_safe_resize_2)
{
    bool thrown = false;
    FooThrowing::reset_counters(12);
    try {
        yato::vector_3d<FooThrowing> v;
        v.resize(yato::dims(3, 3, 3), 1);
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

TEST(Yato_VectorND, exception_safe_resize_3)
{
    bool thrown = false;
    FooThrowing::reset_counters(12);
    try {
        yato::vector_3d<FooThrowing> v;
        v.reserve(27);
        v.resize(yato::dims(3, 3, 3), 1);
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

TEST(Yato_VectorND, exception_safe_push)
{
    FooThrowing::reset_counters(9);
    bool thrown = false;
    {
        const yato::vector_nd<FooThrowing, 1> vec1d_1 = { 7, 7 };
        yato::vector_2d<FooThrowing> v = { {1, 2}, {3, 4} };
        try {
            v.push_back(vec1d_1);
        }
        catch(TestError &) {
            // expected exception
            thrown = true;
        }
        catch(...) {
            // error
            throw;
        }
        EXPECT_EQ(2u, v.size(0));
        EXPECT_EQ(2u, v.size(1));
        EXPECT_EQ(1, v[0][0]);
        EXPECT_EQ(2, v[0][1]);
        EXPECT_EQ(3, v[1][0]);
        EXPECT_EQ(4, v[1][1]);
    }
    EXPECT_TRUE(thrown);
    EXPECT_EQ(FooThrowing::ctors, FooThrowing::dtors);
}

TEST(Yato_VectorND, exception_safe_push_2)
{
    FooThrowing::reset_counters(11);
    bool thrown = false;
    {
        const yato::vector_nd<FooThrowing, 1> vec1d_1 = { 7, 7 };
        yato::vector_2d<FooThrowing> v = { {1, 2}, {3, 4} };
        v.reserve(6);
        try {
            v.push_back(vec1d_1);
        }
        catch(TestError &) {
            // expected exception
            thrown = true;
        }
        catch(...) {
            // error
            throw;
        }
        EXPECT_EQ(2u, v.size(0));
        EXPECT_EQ(2u, v.size(1));
        EXPECT_EQ(1, v[0][0]);
        EXPECT_EQ(2, v[0][1]);
        EXPECT_EQ(3, v[1][0]);
        EXPECT_EQ(4, v[1][1]);
    }
    EXPECT_TRUE(thrown);
    EXPECT_EQ(FooThrowing::ctors, FooThrowing::dtors);
}


TEST(Yato_VectorND, exception_safe_insert)
{
    FooThrowing::reset_counters(7);
    bool thrown = false;
    {
        yato::vector_nd<FooThrowing, 2> vec  = { { 1, 1 },{ 3, 3 } };
        yato::vector_nd<FooThrowing, 1> line = { 2, 2 };
        try {
            vec.insert(std::next(vec.begin()), line);
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

TEST(Yato_VectorND, exception_safe_insert_2)
{
    FooThrowing::reset_counters(11);
    bool thrown = false;
    {
        yato::vector_nd<FooThrowing, 2> vec  = { { 1, 1 },{ 3, 3 } };
        yato::vector_nd<FooThrowing, 1> line = { 2, 2 };
        vec.reserve(6);
        try {
            vec.insert(std::next(vec.begin()), line);
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

TEST(Yato_VectorND, exception_safe_insert_3)
{
    FooThrowing::reset_counters(11);
    bool thrown = false;
    {
        yato::vector_nd<FooThrowing, 2> vec  = { { 1, 1 },{ 3, 3 } };
        yato::vector_nd<FooThrowing, 1> line = { 2, 2 };
        vec.reserve(6);
        try {
            vec.insert(vec.cend(), line);
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

TEST(Yato_VectorND, exception_safe_insert_4)
{
    FooThrowing::reset_counters(13);
    bool thrown = false;
    {
        yato::vector_nd<FooThrowing, 2> vec  = { { 1, 1, 1 }, { 3, 3, 3 }, { 4, 4, 4 } };
        yato::vector_nd<FooThrowing, 1> line = { 2, 2, 2 };
        try {
            vec.insert(std::next(vec.begin()), line);
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

TEST(Yato_VectorND, exception_safe_insert_5)
{
    FooThrowing::reset_counters(11);
    bool thrown = false;
    {
        yato::vector_nd<FooThrowing, 2> vec  = { { 1, 1 },{ 3, 3 } };
        yato::vector_nd<FooThrowing, 1> line = { 0, 0 };
        vec.reserve(6);
        try {
            vec.insert(vec.begin(), line);
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

TEST(Yato_VectorND, exception_safe_insert_6)
{
    FooThrowing::reset_counters(17);
    bool thrown = false;
    {
        yato::vector_nd<FooThrowing, 2> vec  = { { 1, 1, 1 },{ 3, 3, 3 } };
        yato::vector_nd<FooThrowing, 1> line = { 2, 2, 2 };
        vec.reserve(9);
        try {
            vec.insert(vec.cend(), line);
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

TEST(Yato_VectorND, exception_safe_insert_7)
{
    FooThrowing::reset_counters(29);
    bool thrown = false;
    {
        yato::vector_nd<FooThrowing, 2> vec1  = { { 1, 1 }, { 2, 2 }, { 3, 3 }, { 4, 4 }, { 5, 5 } };
        yato::vector_nd<FooThrowing, 2> vec2 = { { 10, 10 } , { 11, 11 } };
        vec1.reserve(14);
        try {
            vec1.insert(std::next(vec1.begin(), 2), vec2.begin(), vec2.end());
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

TEST(Yato_VectorND, exception_safe_erase)
{
    FooThrowing::reset_counters(13);
    bool thrown = false;
    {
        yato::vector_nd<FooThrowing, 2> vec1 = { { 1, 1 }, { 4, 4 }, { 5, 5 }, { 2, 2 }, { 3, 3 } };
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

TEST(Yato_VectorND, exception_safe_erase_2)
{
    FooThrowing::reset_counters(11);
    bool thrown = false;
    {
        yato::vector_nd<FooThrowing, 2> vec1 = { { 1, 1 }, { 4, 4 }, { 5, 5 }, { 2, 2 }, { 3, 3 } };
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

TEST(Yato_VectorND, view_from_proxy)
{
    yato::vector_2d<int> vec1 = { { 1, 2, 3 }, { 4, 5, 6 } };

    yato::array_view_1d<int> view2 = vec1[0];
    EXPECT_EQ(3, view2.size());
    EXPECT_EQ(1, view2[0]);
    EXPECT_EQ(2, view2[1]);
    EXPECT_EQ(3, view2[2]);

    view2 = vec1[1];
    EXPECT_EQ(3, view2.size());
    EXPECT_EQ(4, view2[0]);
    EXPECT_EQ(5, view2[1]);
    EXPECT_EQ(6, view2[2]);

    yato::vector_3d<int> vec2 = { { { 1, 2, 3 }, 
                                    { 4, 5, 6 },
                                    { 7, 8, 9 } },
                                  {  { 11, 12, 13 }, 
                                     { 14, 15, 16 },
                                     { 17, 18, 19 } } };

    yato::array_view_2d<int> view4 = vec2[0];
    EXPECT_EQ(3, view4.size(0));
    EXPECT_EQ(3, view4.size(1));
    EXPECT_EQ(3u * sizeof(int), view4.stride(0));
    EXPECT_EQ(1, view4[0][0]);
    EXPECT_EQ(2, view4[0][1]);
    EXPECT_EQ(3, view4[0][2]);
    EXPECT_EQ(4, view4[1][0]);
    EXPECT_EQ(5, view4[1][1]);
    EXPECT_EQ(6, view4[1][2]);
    EXPECT_EQ(7, view4[2][0]);
    EXPECT_EQ(8, view4[2][1]);
    EXPECT_EQ(9, view4[2][2]);

    view4 = vec2[1];
    EXPECT_EQ(3, view4.size(0));
    EXPECT_EQ(3, view4.size(1));
    EXPECT_EQ(3u * sizeof(int), view4.stride(0));
    EXPECT_EQ(11, view4[0][0]);
    EXPECT_EQ(12, view4[0][1]);
    EXPECT_EQ(13, view4[0][2]);
    EXPECT_EQ(14, view4[1][0]);
    EXPECT_EQ(15, view4[1][1]);
    EXPECT_EQ(16, view4[1][2]);
    EXPECT_EQ(17, view4[2][0]);
    EXPECT_EQ(18, view4[2][1]);
    EXPECT_EQ(19, view4[2][2]);
}

template <typename Impl_>
static void foo_cont(const yato::const_container_nd<int, 2, Impl_> & c)
{
    EXPECT_EQ(8, c.stride(0));
    EXPECT_EQ(42, c[1][1]);
}

TEST(Yato_VectorND, container)
{
    yato::vector_2d<int> v = {{1, 2}, {3, 42}};
    foo_cont(v);
}
