#include "gtest/gtest.h"

#include <atomic>
#include <future>
#include <string>

#include <yato/atomic_attributes.h>

// is_copy_constructible is broken in MSVC2013 
// https://connect.microsoft.com/VisualStudio/feedback/details/802032 
#ifndef YATO_MSVC_2013

namespace
{

    class Foo
        : public yato::atomic_attributes<>
    {
    public:
        Foo()
        {
            register_attribute<int>("a", -1);
            register_attribute<float>("c", -1.0f);
        }

        Foo(const Foo & other) = default;

        Foo & operator = (const Foo & other) = default;
    };

}


TEST(Yato_AtomicAttributes, common)
{
    Foo foo;
    EXPECT_TRUE(foo.has_attribute("a"));
    EXPECT_FALSE(foo.has_attribute("b"));
    EXPECT_TRUE(foo.has_attribute("c"));

    EXPECT_TRUE(foo.set_attribute("a", 1));
    EXPECT_FALSE(foo.set_attribute("b", std::string("some")));
    EXPECT_TRUE(foo.set_attribute("c", 1.0f));

    EXPECT_TRUE(foo.has_attribute("a"));
    EXPECT_FALSE(foo.has_attribute("b"));
    EXPECT_TRUE(foo.has_attribute("c"));

    Foo foo2{ foo };
    EXPECT_TRUE(foo2.has_attribute("a"));
    EXPECT_FALSE(foo2.has_attribute("b"));
    EXPECT_TRUE(foo2.has_attribute("c"));
  
    Foo foo3{ std::move(foo2) };
    EXPECT_TRUE(foo3.has_attribute("a"));
    EXPECT_FALSE(foo3.has_attribute("b"));
    EXPECT_TRUE(foo3.has_attribute("c"));

    EXPECT_NO_THROW(foo.get_attribute_as<int>("a"));
    EXPECT_NO_THROW(foo.get_attribute_as<float>("c"));
    EXPECT_THROW(foo.get_attribute_as<char>("a"), yato::bad_attribute);
    EXPECT_THROW(foo.get_attribute_as<char>("b"), yato::bad_attribute);
}

namespace
{
    enum class BarAttr
    {
        eInt,
        eChar,
        eNone
    };

    class Bar
        : public yato::atomic_attributes<BarAttr>
    {
    public:
        Bar()
        {
            register_attribute<int>(BarAttr::eInt, 0);
            register_attribute<char>(BarAttr::eChar, '\0');
        }
    };

}

TEST(Yato_AtomicAttributes, common2)
{
    Bar b;

    EXPECT_TRUE(b.is_valide_attribute(BarAttr::eInt));
    EXPECT_TRUE(b.is_valide_attribute(BarAttr::eChar));

    EXPECT_TRUE(b.has_attribute(BarAttr::eInt));
    EXPECT_TRUE(b.has_attribute(BarAttr::eChar));

    EXPECT_EQ(0,  b.get_attribute_as<int>(BarAttr::eInt, -1));
    EXPECT_EQ('\0', b.get_attribute_as<char>(BarAttr::eChar, 'x'));
    EXPECT_EQ(nullptr, b.get_attribute_as<void*>(BarAttr::eNone, nullptr));

    b.set_attribute(BarAttr::eInt, 2);
    b.set_attribute(BarAttr::eChar, 'a');

    EXPECT_TRUE(b.has_attribute(BarAttr::eInt));
    EXPECT_TRUE(b.has_attribute(BarAttr::eChar));

    EXPECT_EQ(2,   b.get_attribute_as<int>(BarAttr::eInt, -1));
    EXPECT_EQ('a', b.get_attribute_as<char>(BarAttr::eChar, 'x'));
}


TEST(Yato_AtomicAttributes, concurrency)
{
    static const int N = 1000;

    for (int iter = 0; iter < 100; ++iter) {
        Bar b;
        auto task1 = std::async(std::launch::async, [&b]() {
            try {
                for (int i = 0; i < N; ++i) {
                    b.set_attribute(BarAttr::eInt, i);
                    if (b.has_attribute(BarAttr::eChar)) {
                        char attr = b.get_attribute_as<char>(BarAttr::eChar, 'X');
                        if (attr != '\0') {
                            EXPECT_EQ('A', attr);
                        }
                    }
                }
            }
            catch (std::exception & e) {
                std::cout << e.what() << std::endl;
                EXPECT_TRUE(false);
            }
            catch (...) {
                EXPECT_TRUE(false);
            }
        });

        auto task2 = std::async(std::launch::async, [&b]() {
            try {
                int prev = -1;
                for (int i = 0; i < N; ++i) {
                    int j = 0;
                    j = b.get_attribute_as<int>(BarAttr::eInt, -1);
                    EXPECT_TRUE(j >= prev);
                    prev = j;

                    if (j % 2 == 0) {
                        b.set_attribute(BarAttr::eChar, 'A');
                    }
                }
            }
            catch (std::exception & e) {
                std::cout << e.what() << std::endl;
                EXPECT_TRUE(false);
            }
            catch (...) {
                EXPECT_TRUE(false);
            }
        });

        task1.wait();
        task2.wait();
    }
}

#endif
