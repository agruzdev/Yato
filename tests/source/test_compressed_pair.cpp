/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2020 Alexey Gruzdev
 */

#include "gtest/gtest.h"

#include <yato/compressed_pair.h>

namespace
{
    struct EmptyPod
    { };

    class EmptyClass
    {
    public:
        EmptyClass(int) {}
        EmptyClass(const EmptyClass&) = delete;

        int get()
        {
            return 42;
        }
    };

    class Foo
    {
    public:
        int32_t x = 0;

        Foo()
        { }

        explicit
        Foo(int32_t x)
            : x(x)
        { }
    };

}

TEST(Yato_ComppressedPair, size)
{
    using t1 = yato::compressed_pair<int, short>;
    static_assert(sizeof(t1) >= sizeof(int) + sizeof(short), "yato::compressed_pair failed");

    using t2 = yato::compressed_pair<EmptyPod, short>;
    static_assert(std::is_empty<EmptyPod>::value && !std::is_final<EmptyPod>::value, "yato::compressed_pair failed");
    static_assert(sizeof(t2) == sizeof(short), "yato::compressed_pair failed");

    using t3 = yato::compressed_pair<EmptyClass, double>;
    static_assert(sizeof(t3) == sizeof(double), "yato::compressed_pair failed");

    using t4 = yato::compressed_pair<EmptyClass, Foo>;
    static_assert(sizeof(t4) == sizeof(Foo), "yato::compressed_pair failed");
}

TEST(Yato_ComppressedPair, full)
{
    yato::compressed_pair<Foo, Foo> full_pair(yato::zero_arg_then_variadic_t{});
    EXPECT_EQ(0, full_pair.first().x);
    EXPECT_EQ(0, full_pair.second().x);
    full_pair.first().x  = 1;
    full_pair.second().x = 2;

    auto tmp = full_pair;
    EXPECT_EQ(1, tmp.first().x);
    EXPECT_EQ(2, tmp.second().x);

    yato::compressed_pair<Foo, Foo> full_pair2(yato::one_arg_then_variadic_t{}, 10);
    EXPECT_EQ(10, full_pair2.first().x);
    EXPECT_EQ(0, full_pair2.second().x);

    yato::compressed_pair<Foo, Foo> full_pair3(yato::one_arg_then_variadic_t{}, 10, 20);
    EXPECT_EQ(10, full_pair3.first().x);
    EXPECT_EQ(20, full_pair3.second().x);
}
 
TEST(Yato_ComppressedPair, compressed)
{
    yato::compressed_pair<EmptyClass, Foo> compr_pair(yato::one_arg_then_variadic_t{}, 1, 2);
    EXPECT_EQ(42, compr_pair.first().get());
    EXPECT_EQ(2, compr_pair.second().x);

    compr_pair.second().x = 4;

    yato::compressed_pair<EmptyClass, Foo> compr_pair2(yato::one_arg_then_variadic_t{}, 10);
    EXPECT_EQ(42, compr_pair2.first().get());
    EXPECT_EQ(0, compr_pair2.second().x);

    yato::compressed_pair<EmptyClass, Foo> compr_pair3(yato::one_arg_then_variadic_t{}, 10, 20);
    EXPECT_EQ(42, compr_pair3.first().get());
    EXPECT_EQ(20, compr_pair3.second().x);
}

TEST(Yato_ComppressedPair, get_as)
{
    yato::compressed_pair<int, float> pair1;
    pair1.get_as<int>() = 1;
    pair1.get_as<float>() = 2.0f;

    yato::compressed_pair<EmptyClass, Foo> compr_pair(yato::one_arg_then_variadic_t{}, 1, 2);
    EXPECT_EQ(42, compr_pair.get_as<EmptyClass>().get());
    EXPECT_EQ(2, compr_pair.get_as<Foo>().x);
}

TEST(Yato_ComppressedPair, std_get)
{
    using std::get;

    yato::compressed_pair<int, float> pair1;
    get<0>(pair1) = 1;
    get<1>(pair1) = 2.0f;

    yato::compressed_pair<EmptyClass, Foo> compr_pair(yato::one_arg_then_variadic_t{}, 1, 2);
    EXPECT_EQ(42, get<EmptyClass>(compr_pair).get());
    EXPECT_EQ(2, get<Foo>(compr_pair).x);
}

TEST(Yato_ComppressedPair, to_tuple)
{
    yato::compressed_pair<int, long> pair1(yato::one_arg_then_variadic_t{}, 1, 2);
    std::tuple<int, long> t1 = pair1;
    EXPECT_EQ(1, std::get<0>(t1));
    EXPECT_EQ(2, std::get<1>(t1));

    yato::compressed_pair<EmptyPod, Foo> compr_pair(yato::zero_arg_then_variadic_t{}, 2);
    std::pair<EmptyPod, Foo> t2 = compr_pair;
    EXPECT_EQ(2, t2.second.x);
}
