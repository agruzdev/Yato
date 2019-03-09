/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2019 Alexey Gruzdev
 */

#include "gtest/gtest.h"

#include <cmath>

#include <yato/lookup_table.h>

namespace
{
    int32_t lineaar_foo(int32_t x)
    {
        return x / 2 - 2;
    }
}

TEST(Yato_LookupTable, common)
{
    auto lut = yato::lookup_table<int32_t, int32_t, 128>::create(&lineaar_foo, 0, 127);

    auto y0 = lut->get(0);
    EXPECT_EQ(-2, y0);

    auto y1 = lut->get(1);
    EXPECT_EQ(-2, y1);

    auto y2 = lut->get(2);
    EXPECT_EQ(-1, y2);

    auto y3 = lut->get(3);
    EXPECT_EQ(-1, y3);

    auto y4 = lut->get(4);
    EXPECT_EQ(0, y4);

    auto y8 = lut->get(8);
    EXPECT_EQ(2, y8);

    auto y100 = lut->get(100);
    EXPECT_EQ(48, y100);

    auto y127 = lut->get(127);
    EXPECT_EQ(61, y127);
}

namespace
{
    float intSqrt(int32_t x) {
        return sqrt(static_cast<float>(x));
    }
}

TEST(Yato_LookupTable, float_from_int_function) 
{
    auto sqrtLut = yato::lookup_table<float, int32_t, 128, yato::lut_linear_interpolator<float>>::create(&::intSqrt, 1, 128);

    float y = sqrtLut->get(1);
    EXPECT_FLOAT_EQ(1.0f, y);


    y = sqrtLut->get(2);
    EXPECT_NEAR(1.4f, y, 0.1f);

    y = sqrtLut->get(4);
    EXPECT_FLOAT_EQ(2.0f, y);

    y = sqrtLut->get(16);
    EXPECT_FLOAT_EQ(4.0f, y);

    y = sqrtLut->get(10);
    EXPECT_NEAR(3.16f, y, 0.1f);

    y = sqrtLut->get(100);
    EXPECT_NEAR(10.0f, y, 0.001f);

    y = sqrtLut->get(128);
    EXPECT_NEAR(11.3f, y, 0.1f);
}

TEST(Yato_LookupTable, float_from_float_function) 
{
    const float lim = 3.1415f;
    auto sinLut = yato::lookup_table<float, float, 512, yato::lut_nearest_interpolator<float>>::create(&::sinf, 0.0f, lim);

    constexpr size_t N = 400;
    const float EPS = lim / decltype(sinLut)::element_type::table_size;
    for(size_t i = 0; i <= N; ++i) {
        const float x = i / static_cast<float>(N) * lim;
        const float y = sinLut->get(x);
        EXPECT_NEAR(y, sinf(x), EPS);
    }
}

TEST(Yato_LookupTable, float_from_float_function_2)
{
    const float lim = 3.1415f;
    auto cosLut = yato::lookup_table<float, float, 256, yato::lut_linear_interpolator<float>>::create(&::cosf, 0.0f, lim);

    constexpr size_t N = 400;
    const float EPS = lim / decltype(cosLut)::element_type::table_size;
    for (size_t i = 0; i <= N; ++i) {
        const float x = i / static_cast<float>(N) * lim;
        const float y = cosLut->get(x);
        EXPECT_NEAR(y, cosf(x), EPS);
    }
}

