#include "gtest/gtest.h"

#include <yato/type_match.h>

TEST(Yato_TypeMatch, intergral_type)
{
    auto res = yato::match(
        [](float)  { return 1; },
        [](int)    { return 2; },
        [](double) { return 3; }
    )(1);
    EXPECT_EQ(2, res);
}

TEST(Yato_TypeMatch, reference)
{
    float x{ 1.0f };
    float & rx{ x };
    auto res = yato::match(
        [](float &)  { return 1; },
        [](int &)    { return 2; },
        [](double &) { return 3; }
    )(rx);
    EXPECT_EQ(1, res);
}

TEST(Yato_TypeMatch, reference_2)
{
    auto res = yato::match(
        [](float &)  { return 1; },
        [](int &)    { return 2; },
        [](double &) { return 3; },
        [](int)      { return 4; }
    )(3);
    EXPECT_EQ(4, res);
}

TEST(Yato_TypeMatch, rvalue_reference)
{
    float x{ 1.0f };
    auto res = yato::match(
        [](float &&)  { return 1; },
        [](int &&)    { return 2; },
        [](double &&) { return 3; },
        [](float &)   { return 4; },
        [](int &)     { return 5; },
        [](double &)  { return 6; }
    )(std::move(x));
    EXPECT_EQ(1, res);
}

TEST(Yato_TypeMatch, const_reference)
{
    double x{ 1.0 };
    const double & rx{ x };
    auto res = yato::match(
        [](const float &)  { return 1; },
        [](const int &)    { return 2; },
        [](const double &) { return 3; }
    )(rx);
    EXPECT_EQ(3, res);
}

TEST(Yato_TypeMatch, pointer)
{
    double x{ 1.0 };
    double* px{ &x };
    auto res = yato::match(
        [](const float *)  { return 1; },
        [](const int)      { return 2; },
        [](const void *)   { return 3; },
        [](const double *) { return 4; }
    )(px);
    EXPECT_EQ(4, res);
}


