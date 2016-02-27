#include "gtest/gtest.h"

#include <yato/tuple.h>

namespace
{
    template <typename _T>
    struct add_one
    {
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
        auto t2 = yato::tuple_transform<add_one>(t1);
        EXPECT_EQ(1, std::get<0>(t2));
        EXPECT_EQ('b', std::get<1>(t2));
        EXPECT_EQ(2.0f, std::get<2>(t2));
    }
    {
        auto t2 = yato::tuple_transform<add_one>(std::make_tuple(1.0));
        EXPECT_EQ(2.0, std::get<0>(t2));
    }
#ifndef YATO_MSVC_2013
    {
        constexpr std::tuple<int, float> t3(10, 10.0f);
        constexpr std::tuple<int, float> t4 = yato::tuple_transform<add_one>(t3);
        static_assert(11 == std::get<0>(t4), "tuple_transform fail");
        static_assert(11.0f == std::get<1>(t4), "tuple_transform fail");
    }
    {
        constexpr std::tuple<int> t1{ yato::tuple_transform<add_one>(std::tuple<int>{0}) };
        static_assert(1 == std::get<0>(t1), "tuple_transform fail");
    }
#endif
}

namespace
{
    template<typename _T1, typename _T2>
    struct plus
    {
        YATO_CONSTEXPR_FUNC
        auto operator()(const _T1 & x, const _T2 & y) const
            -> decltype(x + y)
        {
            return x + y;
        }
    };
}

TEST(Yato_Tuple, tuple_transform_2)
{
    {
        auto t1 = std::make_tuple(-1, 20U, 1.0f);
        auto t2 = std::make_tuple(1U, 10, 41.0);
        auto t3 = yato::tuple_transform<plus>(t1, t2);
        EXPECT_EQ(0, std::get<0>(t3));
        EXPECT_EQ(30, std::get<1>(t3));
        EXPECT_EQ(42.0, std::get<2>(t3));
    }
#ifndef YATO_MSVC_2013
    {
        constexpr std::tuple<int, float, float>  t1(-1, 20.0f, 1.0f);
        constexpr std::tuple<long, int, double> t2(1, 10, 41.0);
        constexpr std::tuple<long, float, double> t3 = yato::tuple_transform<plus>(t1, t2);
        static_assert(0 == std::get<0>(t3), "tuple_transform fail");
        static_assert(30.0f == std::get<1>(t3), "tuple_transform fail");
        static_assert(42.0 == std::get<2>(t3), "tuple_transform fail");
    }
#endif
}

namespace
{
    template <typename _T>
    struct increment
    {
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
        yato::tuple_for_each<increment>(t1);
        EXPECT_EQ(1, std::get<0>(t1));
        EXPECT_EQ('b', std::get<1>(t1));
        EXPECT_EQ(2.0f, std::get<2>(t1));
    }
}

namespace
{
    template <typename _T>
    struct positive
    {
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
        EXPECT_TRUE(yato::tuple_all_of<positive>(t1));
    }
#ifndef YATO_MSVC_2013
    {
        constexpr std::tuple<int, float> t1(1, 2.0f);
        static_assert(true == yato::tuple_all_of<positive>(t1), "tuple_all_of fail");
        constexpr std::tuple<int, float> t2(1, -2.0f);
        static_assert(false == yato::tuple_all_of<positive>(t2), "tuple_all_of fail");
    }
#endif
}

TEST(Yato_Tuple, tuple_any_of)
{
    {
        auto t1 = std::make_tuple(1, 1.0f, 42.0, 8U);
        EXPECT_TRUE(yato::tuple_any_of<positive>(t1));
    }
#ifndef YATO_MSVC_2013
    {
        constexpr std::tuple<int, float> t1(1, 2.0f);
        static_assert(true == yato::tuple_any_of<positive>(t1), "tuple_any_of fail");
        constexpr std::tuple<int, float> t2(1, -2.0f);
        static_assert(true == yato::tuple_any_of<positive>(t2), "tuple_any_of fail");
        constexpr std::tuple<int, float> t3(-1, -2.0f);
        static_assert(false == yato::tuple_any_of<positive>(t3), "tuple_any_of fail");
    }
#endif
}
