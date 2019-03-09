/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2019 Alexey Gruzdev
 */

#include "gtest/gtest.h"

#include <yato/any_ptr.h>


TEST(Yato_AnyPtr, common)
{
    int x = 1;
    yato::any_ptr px = &x;

    EXPECT_TRUE(px);
    EXPECT_EQ(std::type_index(typeid(int*)), std::type_index(px.type()));
    EXPECT_NE(nullptr, px.get_as<int*>());
    EXPECT_EQ(nullptr, px.get_as<float*>());
    //EXPECT_EQ(nullptr, px.get_as<int>()); // compilation error
    EXPECT_EQ(1, *(px.get_as<int*>()));

}

TEST(Yato_AnyPtr, common2)
{
    int x = 2;

    yato::any_ptr px = static_cast<int*>(&x);
    yato::any_ptr cpx = static_cast<const int*>(&x);
    yato::any_ptr vpx = static_cast<volatile int*>(&x);
    yato::any_ptr cvpx = static_cast<const volatile int*>(&x);

    EXPECT_EQ(std::type_index(typeid(int*)), std::type_index(px.type()));
    EXPECT_NE(std::type_index(typeid(const int*)), std::type_index(px.type()));
    EXPECT_NE(std::type_index(typeid(volatile int*)), std::type_index(px.type()));
    EXPECT_NE(std::type_index(typeid(const volatile int*)), std::type_index(px.type()));

    EXPECT_NE(std::type_index(typeid(int*)), std::type_index(cpx.type()));
    EXPECT_EQ(std::type_index(typeid(const int*)), std::type_index(cpx.type()));
    EXPECT_NE(std::type_index(typeid(volatile int*)), std::type_index(cpx.type()));
    EXPECT_NE(std::type_index(typeid(const volatile int*)), std::type_index(cpx.type()));

    EXPECT_NE(std::type_index(typeid(int*)), std::type_index(vpx.type()));
    EXPECT_NE(std::type_index(typeid(const int*)), std::type_index(vpx.type()));
    EXPECT_EQ(std::type_index(typeid(volatile int*)), std::type_index(vpx.type()));
    EXPECT_NE(std::type_index(typeid(const volatile int*)), std::type_index(vpx.type()));

    EXPECT_NE(std::type_index(typeid(int*)), std::type_index(cvpx.type()));
    EXPECT_NE(std::type_index(typeid(const int*)), std::type_index(cvpx.type()));
    EXPECT_NE(std::type_index(typeid(volatile int*)), std::type_index(cvpx.type()));
    EXPECT_EQ(std::type_index(typeid(const volatile int*)), std::type_index(cvpx.type()));

    EXPECT_TRUE(nullptr != px.get_as<int*>());
    EXPECT_TRUE(nullptr == px.get_as<const int*>());
    EXPECT_TRUE(nullptr == px.get_as<volatile int*>());
    EXPECT_TRUE(nullptr == px.get_as<const volatile int*>());

    EXPECT_TRUE(nullptr == cpx.get_as<int*>());
    EXPECT_TRUE(nullptr != cpx.get_as<const int*>());
    EXPECT_TRUE(nullptr == cpx.get_as<volatile int*>());
    EXPECT_TRUE(nullptr == cpx.get_as<const volatile int*>());

    EXPECT_TRUE(nullptr == vpx.get_as<int*>());
    EXPECT_TRUE(nullptr == vpx.get_as<const int*>());
    EXPECT_TRUE(nullptr != vpx.get_as<volatile int*>());
    EXPECT_TRUE(nullptr == vpx.get_as<const volatile int*>());

    EXPECT_TRUE(nullptr == cvpx.get_as<int*>());
    EXPECT_TRUE(nullptr == cvpx.get_as<const int*>());
    EXPECT_TRUE(nullptr == cvpx.get_as<volatile int*>());
    EXPECT_TRUE(nullptr != cvpx.get_as<const volatile int*>());

}
