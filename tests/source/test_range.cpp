#include "gtest/gtest.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <memory>
#include <vector>

#include <yato/types.h>
#include <yato/range.h>

TEST(Yato_Range, range)
{
    std::vector<int> vec = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    auto vecRange = yato::range<std::vector<int>::iterator>(vec.begin(), vec.end());
    EXPECT_TRUE(vec.begin() == vecRange.begin());
    EXPECT_TRUE(vec.end() == vecRange.end());
    EXPECT_TRUE(vecRange.distance() == 10);
    EXPECT_FALSE(vecRange.empty());
    EXPECT_TRUE(0 == *(vecRange.head()));
    EXPECT_TRUE(1 == *(vecRange.tail().head()));
    EXPECT_TRUE(2 == *(vecRange.tail().tail().head()));
}

TEST(Yato_Range, make_range)
{
    std::vector<int> vec(10);
    auto vecRange = yato::make_range(vec.begin(), vec.end());
    EXPECT_TRUE(vec.begin() == vecRange.begin());
    EXPECT_TRUE(vec.end() == vecRange.end());
    auto vecRangeConst = yato::make_range(vec.cbegin(), vec.cend());
    EXPECT_TRUE(vec.cbegin() == vecRangeConst.begin());
    EXPECT_TRUE(vec.cend() == vecRangeConst.end());
}

TEST(Yato_Range, numeric_range_1)
{
    int i = 0;
    for (int x : yato::numeric_range(0, 100)) {
        EXPECT_TRUE(x == i++);
    }
}

TEST(Yato_Range, numeric_range_2)
{
    size_t i = 0;
    for (size_t x : yato::numeric_range(100U)) {
        EXPECT_TRUE(x == i++);
    }

    int j = 0;
    for (int x : yato::numeric_range(100)) {
        EXPECT_TRUE(x == j++);
    }
}

#if defined(YATO_MSVC_2015) || (__cplusplus >= 201400L)
TEST(Yato_Range, numeric_range_3)
{
    using namespace yato::literals;
    yato::uint8_t i = 0;
    for (auto x : yato::numeric_range(100_u8)) {
        EXPECT_TRUE(x == i++);
    }
}
#endif

TEST(Yato_Range, make_range_2)
{
    class A
    {
        std::vector<int> * m_data;
    public:
        A(std::vector<int> * data)
            : m_data(data)
        { }

        yato::range<typename std::vector<int>::iterator> range()
        {
            return yato::make_range(m_data->begin(), m_data->end());
        }

        yato::range<typename std::vector<int>::const_iterator> crange() const 
        {
            return yato::make_range(m_data->cbegin(), m_data->cend());
        }

        std::vector<int>::iterator begin()
        {
            return m_data->begin();
        }

        std::vector<int>::iterator end()
        {
            return m_data->end();
        }

        std::vector<int>::const_iterator begin() const
        {
            return m_data->cbegin();
        }

        std::vector<int>::const_iterator end() const
        {
            return m_data->cend();
        }

        std::vector<int>::const_iterator cbegin() const
        {
            return m_data->cbegin();
        }

        std::vector<int>::const_iterator cend() const
        {
            return m_data->cend();
        }
    };

    std::vector<int> vec;
    vec.assign(std::rand() % 100, std::rand() % 100);

    A a = A(&vec);
    auto r = yato::make_range<A&>(a);
    (void)r;

    auto it = vec.begin();
    for (int & x : yato::make_range(a)) {
        EXPECT_TRUE(x == *it++);
    }


    const A& c_a = a;
    it = vec.begin();
    for (const int & x : yato::make_crange(c_a)) {
        EXPECT_TRUE(x == *it++);
    }
}

TEST(Yato_Range, reverse)
{
    std::vector<int> v = { 1, 2, 3, 4 };
    auto r1 = yato::make_range(v);

    int i = 4;
    for (auto it = r1.rbegin(); it != r1.rend(); ++it) {
        EXPECT_EQ(i--, *it);
    }

    int j = 4;
    auto r2 = yato::make_range(v).reverse();
    for (auto it = r2.begin(); it != r2.end(); ++it) {
        EXPECT_EQ(j--, *it);
    }
}

TEST(Yato_Range, map)
{
    std::vector<int> v = { 1, 2, 3, 4 };

    int i = 1;
    for (int x : yato::make_range(v).map([](int y) { return 2 * y; })) {
        EXPECT_EQ(x, i++ * 2);
    }

    const std::vector<int> u = { 1, 2, 3, 4 };
    auto r = yato::make_range(u).map([](int y) { return y + 1; });
    std::vector<int> w;
    for (int x : r) {
        w.push_back(x);
    }
    EXPECT_EQ(w, (std::vector<int>{ 2, 3, 4, 5}));
}

#ifndef YATO_MSVC_2013
TEST(Yato_Range, filter)
{
    std::vector<int> v = { 1, 2, 3, 4, 5, 6, 7 };

    std::vector<int> u;
    for (int x : yato::make_range(v).filter([](int y) { return (y & 1) == 0; })) {
        u.push_back(x);
    }
    EXPECT_EQ(u, (std::vector<int>{ 2, 4, 6 }));
}
#endif

TEST(Yato_Range, zip)
{
    std::vector<int> v = {  1,  2,  3,  4 };
    std::vector<int> u = { -1, -2, -3, -4 };
    
    auto r1 = yato::make_range(v).zip(yato::make_range(u));
    for (const auto & t : r1) {
        EXPECT_EQ(std::get<0>(t), -std::get<1>(t));
    }

    auto r2 = yato::make_range(v).zip(yato::make_range(u), yato::numeric_range(1U, 5U));
    for (const auto & t : r2) {
        EXPECT_EQ(std::get<0>(t), -std::get<1>(t));
        EXPECT_EQ(std::get<0>(t), static_cast<int>(std::get<2>(t)));
    }
}

TEST(Yato_Range, fold)
{
    std::vector<int> v = { 1, 2, 3, 4 };
    long s1 = yato::make_range(v).fold_left(std::plus<long>(), static_cast<long>(0));
    EXPECT_EQ(10, s1);
    long s2 = yato::make_range(v).fold_right(std::multiplies<long>(), static_cast<long>(1));
    EXPECT_EQ(24, s2);
}

#ifndef YATO_MSVC_2013
TEST(Yato_Range, superposition)
{
    //Count number of numbers '1' after rounding
    std::vector<float> v = {4.1f, 0.0f, -2.4f, 4.9f, 1.9f, 1.1f, 4.0f, 0.4f, -5.0f, 6.1f, 2.4f, 1.0f, 5.3f, 0.9f, 1.0f, 0.0f, 5.4f, -1.1f, 5.0f};

    size_t num = yato::make_range(v).map([](float y)->int {return static_cast<int>(std::round(y)); }).filter([](int x) {return x == 1; }).fold_left(std::plus<size_t>(), static_cast<size_t>(0));
    EXPECT_EQ(4U, num);
}
#endif

TEST(Yato_Range, convertion)
{
    std::vector<float> v = { 1.1f, 2.0f, 3.5f, 4.8f };

    auto r = yato::make_range(v).map([](float y)->int {return static_cast<int>(std::round(y)); });
    std::vector<int> u = r.to_vector();
    EXPECT_EQ(u, (std::vector<int>{ 1, 2, 4, 5 }));
    std::list<int> l = r.to_list();
    EXPECT_EQ(l, (std::list<int>{ 1, 2, 4, 5 }));
    auto s = r.to_set<std::greater<int>>();
    EXPECT_EQ(s, (std::set<int, std::greater<int>>{ 1, 2, 4, 5 }));
}
