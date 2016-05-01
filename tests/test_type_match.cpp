#include "gtest/gtest.h"

#include <yato/type_match.h>


TEST(Yato_TypeMatch, intergral_type)
{
    auto res = yato::match(1)(
        [](float)  { return 1; },
        [](int)    { return 2; },
        [](double) { return 3; }
    );
    EXPECT_EQ(2, res);
}

TEST(Yato_TypeMatch, reference)
{
    float x{ 1.0f };
    float & rx{ x };
    auto res = yato::match(rx)(
        [](float &)  { return 1; },
        [](int &)    { return 2; },
        [](double &) { return 3; }
    );
    EXPECT_EQ(1, res);
}

TEST(Yato_TypeMatch, const_reference)
{
    double x{ 1.0 };
    const double & rx{ x };
    auto res = yato::match(rx)(
        [](const float &)  { return 1; },
        [](const int &)    { return 2; },
        [](const double &) { return 3; }
    );
    EXPECT_EQ(3, res);
}

TEST(Yato_TypeMatch, pointer)
{
    double x{ 1.0 };
    double* px{ &x };
    auto res = yato::match(px)(
        [](const float *)  { return 1; },
        [](const int)      { return 2; },
        [](const void *)   { return 3; },
        [](const double *) { return 4; }
    );
    EXPECT_EQ(4, res);
}
