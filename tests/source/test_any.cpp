/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2020 Alexey Gruzdev
 */

#include "gtest/gtest.h"

#include <atomic>
#include <yato/any.h>

namespace
{
    
    class Foo
    { };

    class Bar
    {
    public:
        Bar() {}
        Bar(const Bar&) = delete;
        Bar& operator=(const Bar&) = delete;
        Bar(Bar&&) {};
        Bar& operator=(Bar&&) { return *this; };
    };

    template <typename T>
    void TestFunction(const yato::any & any)
    {
        if (any.type() == typeid(int)) {
            EXPECT_TRUE((std::is_same<T, int>::value));
        }
        else if (any.type() == typeid(float)) {
            EXPECT_TRUE((std::is_same<T, float>::value));
        }
        else if (any.type() == typeid(Foo)) {
            EXPECT_TRUE((std::is_same<T, Foo>::value));
        }
        else if (any.type() == typeid(Bar)) {
            EXPECT_TRUE((std::is_same<T, Bar>::value));
        }
        else {
            // Unknown
        }
    }

    template <typename T>
    void TestFunction2(const yato::any & any)
    {
        if (any.is_type<int>()) {
            EXPECT_TRUE((std::is_same<T, int>::value));
            EXPECT_TRUE(static_cast<bool>(any.get_opt<int>()));
        }
        else if (any.is_type<float>()) {
            EXPECT_TRUE((std::is_same<T, float>::value));
            EXPECT_TRUE(static_cast<bool>(any.get_opt<float>()));
        }
        else if (any.is_type<Foo>()) {
            EXPECT_TRUE((std::is_same<T, Foo>::value));
            EXPECT_TRUE(static_cast<bool>(any.get_opt<Foo>()));
        }
        else if (any.is_type<Bar>()) {
            EXPECT_TRUE((std::is_same<T, Bar>::value));
        }
        else {
            // Unknown
        }
    }
}

TEST(Yato_Any, common)
{
    TestFunction<int>(yato::any(1));
    TestFunction<int>(yato::any(1U));
    TestFunction<float>(yato::any(1.0f));
    TestFunction<float>(yato::any(1.0));
    TestFunction<double>(yato::any(1.0));
    TestFunction<Foo>(yato::any(Foo{}));

    // is_copy_constructible is broken in MSVC2013 
    // https://connect.microsoft.com/VisualStudio/feedback/details/802032 
#ifndef YATO_MSVC_2013
    TestFunction<Bar>(yato::any(Bar{}));
    TestFunction<std::unique_ptr<Foo>>(yato::any(std::make_unique<Foo>()));
#endif

    EXPECT_TRUE(static_cast<bool>(yato::any(1)));
    EXPECT_FALSE(static_cast<bool>(yato::any()));

    EXPECT_TRUE(static_cast<bool>(yato::make_any<float>(1.0f)));
}

TEST(Yato_Any, common_2)
{
    TestFunction2<int>(yato::any(1));
    TestFunction2<int>(yato::any(1U));
    TestFunction2<float>(yato::any(1.0f));
    TestFunction2<float>(yato::any(1.0));
    TestFunction2<double>(yato::any(1.0));
    TestFunction2<Foo>(yato::any(Foo{}));
}

TEST(Yato_Any, bad_any_cast)
{
    yato::any anyInt(1);
    EXPECT_THROW(anyInt.get_as<float>(), yato::bad_any_cast);
    EXPECT_THROW(anyInt.get_as<short>(), yato::bad_any_cast);
    EXPECT_NO_THROW(anyInt.get_as<int>());
}

// is_copy_constructible is broken in MSVC2013 
// https://connect.microsoft.com/VisualStudio/feedback/details/802032 
#ifndef YATO_MSVC_2013
TEST(Yato_Any, atomic)
{
    yato::any a;
    a.emplace<std::atomic<int>>(1);
    EXPECT_EQ(1, a.get_as<std::atomic<int>>().load());

    yato::any a2(yato::in_place_type_t<std::atomic<float>> (), 2.0f);
    EXPECT_EQ(2.0f, a2.get_as<std::atomic<float>>().load());
}
#endif
