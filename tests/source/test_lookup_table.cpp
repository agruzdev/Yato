#include "gtest/gtest.h"

#include <memory>
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
    yato::lookup_table<int32_t, int32_t, 128> lut;
    lut.init(&lineaar_foo, 0, 127);

    auto y0 = lut.get(0);
    EXPECT_EQ(-2, y0);

    auto y1 = lut.get(1);
    EXPECT_EQ(-2, y1);

    auto y2 = lut.get(2);
    EXPECT_EQ(-1, y2);

    auto y3 = lut.get(3);
    EXPECT_EQ(-1, y3);

    auto y4 = lut.get(4);
    EXPECT_EQ(0, y4);

    auto y8 = lut.get(8);
    EXPECT_EQ(2, y8);

    auto y100 = lut.get(100);
    EXPECT_EQ(48, y100);

    auto y127 = lut.get(127);
    EXPECT_EQ(61, y127);
}

namespace
{
    float intSqrt(int32_t x) {
        return sqrt(static_cast<float>(x));
    }
}

TEST(Yato_LookupTable, float_function) 
{
    yato::lookup_table <float, int32_t, 128, yato::lut_linear_interpolator<float>> sqrtLut;
    sqrtLut.init(&::intSqrt, 1, 128);
    
    float y = sqrtLut.get(1);
    EXPECT_FLOAT_EQ(1.0f, y);


    y = sqrtLut.get(2);
    EXPECT_NEAR(1.4f, y, 0.1f);

    y = sqrtLut.get(4);
    EXPECT_FLOAT_EQ(2.0f, y);

    y = sqrtLut.get(16);
    EXPECT_FLOAT_EQ(4.0f, y);

    y = sqrtLut.get(10);
    EXPECT_NEAR(3.16f, y, 0.1f);

    y = sqrtLut.get(100);
    EXPECT_NEAR(10.0f, y, 0.001f);

    y = sqrtLut.get(128);
    EXPECT_NEAR(11.3f, y, 0.1f);
}
