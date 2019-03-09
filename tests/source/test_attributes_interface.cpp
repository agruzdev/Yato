/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2019 Alexey Gruzdev
 */

#include "gtest/gtest.h"

#include <array>
#include <atomic>
#include <future>
#include <string>

#include <yato/attributes_interface.h>

namespace
{

    class Foo
        : public yato::attributes_map<>
    { };

    enum BarAttributes
    {
        e1, 
        e2
    };

    class Bar
        : public yato::attributes_map<BarAttributes>
    { };
}


TEST(Yato_Attributes, common)
{
    Foo foo;
    foo.set_attribute("a", 1);
    foo.set_attribute("b", std::string("some"));

    EXPECT_TRUE(foo.has_attribute("a"));
    EXPECT_TRUE(foo.has_attribute("b"));
    EXPECT_FALSE(foo.has_attribute("c"));

    EXPECT_EQ(1, foo.get_attribute_as<int>("a", 0));
    EXPECT_EQ(0, foo.get_attribute_as<float>("a", 0));
    EXPECT_EQ("some", foo.get_attribute_as<std::string>("b", ""));

    EXPECT_THROW(foo.get_attribute("c"), yato::bad_attribute);

    // A mysterious bug here. To be fixed
#ifndef YATO_CLANG
    EXPECT_THROW(foo.get_attribute_as<float>("b"), yato::bad_attribute);
#endif
}

TEST(Yato_Attributes, copy)
{
    Foo f1;
    Foo f2(f1);

    f1 = f2;
    Foo f3(std::move(f2));
    Foo f4{Foo()};

    f3 = std::move(f4);
}

namespace
{
    enum class FooAttr
    {
        e1,
        e2
    };

    class FooAccepts
        : public yato::attributes_map<FooAttr>
    { };

    class FooIngore
        : public yato::ignores_attributes<FooAttr>
    { };
}

TEST(Yato_Attributes, ignore)
{
    FooAccepts fa;
    FooIngore fi;
    std::array<yato::attributes_interface<FooAttr>*, 2> foos;
    foos[0] = &fa;
    foos[1] = &fi;

    for (auto attrI : foos) {
        attrI->set_attribute(FooAttr::e1, 1);
        attrI->set_attribute(FooAttr::e2, 1.0);
    }

    EXPECT_TRUE(fa.has_attribute(FooAttr::e1));
    EXPECT_TRUE(fa.has_attribute(FooAttr::e2));
    EXPECT_EQ(1,   fa.get_attribute_as<int>(FooAttr::e1, 0));
    EXPECT_EQ(1.0, fa.get_attribute_as<double>(FooAttr::e2, 0.0));

    EXPECT_TRUE(fa.is_valide_attribute(FooAttr::e1));
    EXPECT_TRUE(fa.is_valide_attribute(FooAttr::e1));
    EXPECT_FALSE(fi.is_valide_attribute(FooAttr::e1));
    EXPECT_FALSE(fi.is_valide_attribute(FooAttr::e1));
}

TEST(Yato_Attributes, concurrency)
{
    static const int N = 1000;

    for (int iter = 0; iter < 10; ++iter) {
        Bar b;
        auto task1 = std::async(std::launch::async, [&b]() {
            try {
                for (int i = 0; i < N; ++i) {
                    {
                        auto l = b.lock_attributes();
                        b.set_attribute(e1, i);
                    }

                    {
                        auto l = b.lock_attributes();
                        if (b.has_attribute(e2)) {
                            yato::any attr = b.get_attribute(e2);
                            if (attr.type() == typeid(std::string)) {
                                EXPECT_EQ("even", yato::any_cast<std::string>(attr));
                            }
                            else {
                                EXPECT_TRUE(attr.type() == typeid(std::nullptr_t));
                                EXPECT_EQ(nullptr, yato::any_cast<std::nullptr_t>(attr));
                            }
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
                    int j = -1;
                    {
                        auto l = b.lock_attributes();
                        j = b.get_attribute_as<int>(e1, -1);
                    }
                    EXPECT_TRUE(j >= prev);
                    prev = j;

                    {
                        auto l = b.lock_attributes();
                        if (j % 2 == 0) {
                            b.set_attribute(e2, std::string("even"));
                        }
                        else {
                            b.set_attribute(e2, nullptr);
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

        task1.wait();
        task2.wait();
    }
}
