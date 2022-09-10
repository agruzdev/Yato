/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2020 Alexey Gruzdev
 */

#include "gtest/gtest.h"

#include <yato/tuple.h>

namespace
{
    struct add_one
    {
        template <typename _T>
        YATO_CONSTEXPR_FUNC
        _T operator()(const _T & x) const
        {
            return x + 1;
        }
    };
}

TEST(Yato_Tuple, tuple_transform)
{
    {
        auto t1 = std::make_tuple(0, 'a', 1.0f);
        auto t2 = yato::tuple_transform(t1, add_one{});
        EXPECT_EQ(1, std::get<0>(t2));
        EXPECT_EQ('b', std::get<1>(t2));
        EXPECT_EQ(2.0f, std::get<2>(t2));
    }
    {
        auto t2 = yato::tuple_transform(std::make_tuple(1.0), add_one{});
        EXPECT_EQ(2.0, std::get<0>(t2));
    }
#if !YATO_MSVC || (YATO_MSVC >= YATO_MSVC_2013)
    {
        constexpr std::tuple<int, float> t3(10, 10.0f);
        constexpr std::tuple<int, float> t4 = yato::tuple_transform(t3, add_one{});
        static_assert(11 == std::get<0>(t4), "tuple_transform fail");
        static_assert(11.0f == std::get<1>(t4), "tuple_transform fail");
    }
    {
        constexpr std::tuple<int> t1{ yato::tuple_transform(std::tuple<int>{0}, add_one{}) };
        static_assert(1 == std::get<0>(t1), "tuple_transform fail");
    }
#endif
}

namespace
{
    
    struct plus
    {
        template<typename _T1, typename _T2>
        YATO_CONSTEXPR_FUNC
        auto operator()(const _T1 & x, const _T2 & y) const
            -> decltype(x + y)
        {
            return x + y;
        }
    };


    struct product
    {
        template<typename _T1, typename _T2>
        int operator()(const _T1 & x, const _T2 & y, float & sum) const
        {
            sum += x * y;
            return 0;
        }
    };
}

TEST(Yato_Tuple, tuple_transform_2)
{
    {
        auto t1 = std::make_tuple(-1, 20U, 1.0f);
        auto t2 = std::make_tuple(1U, 10, 41.0);
        auto t3 = yato::tuple_transform(t1, t2, plus{});
        EXPECT_EQ(0U, std::get<0>(t3));
        EXPECT_EQ(30U, std::get<1>(t3));
        EXPECT_EQ(42.0, std::get<2>(t3));
    }
#if !YATO_MSVC || (YATO_MSVC >= YATO_MSVC_2013)
    {
        constexpr std::tuple<int, float, float>  t1(-1, 20.0f, 1.0f);
        constexpr std::tuple<long, int, double> t2(1, 10, 41.0);
        constexpr std::tuple<long, float, double> t3 = yato::tuple_transform(t1, t2, plus{});
        static_assert(0 == std::get<0>(t3), "tuple_transform fail");
        static_assert(30.0f == std::get<1>(t3), "tuple_transform fail");
        static_assert(42.0 == std::get<2>(t3), "tuple_transform fail");
    }
#endif
    {
        auto t1 = std::make_tuple(1, 2, 3);
        auto t2 = std::make_tuple(4, 5, 6);
        float r = 0.0f;
        yato::tuple_transform(t1, t2, product{}, r);
        EXPECT_EQ(32.0f, r);
    }
}

namespace
{
    struct increment
    {
        template <typename _T>
        void operator()(_T & x) const
        {
            ++x;
        }
    };
}

TEST(Yato_Tuple, tuple_modify)
{
    {
        auto t1 = std::make_tuple(0, 'a', 1.0f);
        yato::tuple_for_each(t1, increment{});
        EXPECT_EQ(1, std::get<0>(t1));
        EXPECT_EQ('b', std::get<1>(t1));
        EXPECT_EQ(2.0f, std::get<2>(t1));
    }
}

namespace
{
    struct advance
    {
        template <typename _T>
        void operator()(_T & x, int n) const
        {
            x += n;
        }
    };
}

TEST(Yato_Tuple, tuple_modify_2)
{
    {
        auto t1 = std::make_tuple(32, 9.0f);
        yato::tuple_for_each(t1, advance{}, 10);
        EXPECT_EQ(42, std::get<0>(t1));
        EXPECT_EQ(19.0f, std::get<1>(t1));
    }
}

namespace
{
    struct accumulate_sum
    {
        uint32_t sum = 0;

        template <typename T>
        void operator()(const T & x)
        {
            sum += x;
        }
    };
}

TEST(Yato_Tuple, tuple_modify_3)
{
    {
        const auto t1 = std::make_tuple(1, 2u, 3, 4u);
        const auto res = yato::tuple_for_each(t1, accumulate_sum{});
        EXPECT_EQ(10u, res.sum);
    }
}


namespace
{
    struct positive
    {
        template <typename _T>
        YATO_CONSTEXPR_FUNC
        bool operator()(const _T & x) const
        {
            return x > static_cast<_T>(0);
        }
    };
}

TEST(Yato_Tuple, tuple_all_of)
{
    {
        auto t1 = std::make_tuple(1, 1.0f, 42.0, 8U);
        EXPECT_TRUE(yato::tuple_all_of(t1, positive{}));
    }
#if !YATO_MSVC || (YATO_MSVC >= YATO_MSVC_2013)
    {
        constexpr std::tuple<int, float> t1(1, 2.0f);
        static_assert(yato::tuple_all_of(t1, positive{}), "tuple_all_of fail");
        constexpr std::tuple<int, float> t2(1, -2.0f);
        static_assert(!yato::tuple_all_of(t2, positive{}), "tuple_all_of fail");
    }
#endif
}

TEST(Yato_Tuple, tuple_any_of)
{
    {
        auto t1 = std::make_tuple(1, 1.0f, 42.0, 8U);
        EXPECT_TRUE(yato::tuple_any_of(t1, positive{}));
    }
#if !YATO_MSVC || (YATO_MSVC >= YATO_MSVC_2013)
    {
        constexpr std::tuple<int, float> t1(1, 2.0f);
        static_assert(true == yato::tuple_any_of(t1, positive{}), "tuple_any_of fail");
        constexpr std::tuple<int, float> t2(1, -2.0f);
        static_assert(true == yato::tuple_any_of(t2, positive{}), "tuple_any_of fail");
        constexpr std::tuple<int, float> t3(-1, -2.0f);
        static_assert(false == yato::tuple_any_of(t3, positive{}), "tuple_any_of fail");
    }
#endif
}
