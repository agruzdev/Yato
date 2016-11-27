#include "gtest/gtest.h"

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
    inline 
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
}