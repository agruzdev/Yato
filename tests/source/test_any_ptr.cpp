/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2020 Alexey Gruzdev
 */

#include "gtest/gtest.h"

#include <yato/any_ptr.h>


TEST(Yato_AnyPtr, common)
{
    int x = 1;
    yato::any_ptr px = &x;

    EXPECT_TRUE(px);
    EXPECT_EQ(std::type_index(typeid(int*)), std::type_index(px.type()));
    EXPECT_NE(nullptr, px.get<int*>());
    EXPECT_EQ(nullptr, px.get<float*>());
    //EXPECT_EQ(nullptr, px.get<int>()); // compilation error
    EXPECT_EQ(1, *(px.get<int*>()));

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

    EXPECT_TRUE(nullptr != px.get<int*>());
    EXPECT_TRUE(nullptr == px.get<const int*>());
    EXPECT_TRUE(nullptr == px.get<volatile int*>());
    EXPECT_TRUE(nullptr == px.get<const volatile int*>());

    EXPECT_TRUE(nullptr == cpx.get<int*>());
    EXPECT_TRUE(nullptr != cpx.get<const int*>());
    EXPECT_TRUE(nullptr == cpx.get<volatile int*>());
    EXPECT_TRUE(nullptr == cpx.get<const volatile int*>());

    EXPECT_TRUE(nullptr == vpx.get<int*>());
    EXPECT_TRUE(nullptr == vpx.get<const int*>());
    EXPECT_TRUE(nullptr != vpx.get<volatile int*>());
    EXPECT_TRUE(nullptr == vpx.get<const volatile int*>());

    EXPECT_TRUE(nullptr == cvpx.get<int*>());
    EXPECT_TRUE(nullptr == cvpx.get<const int*>());
    EXPECT_TRUE(nullptr == cvpx.get<volatile int*>());
    EXPECT_TRUE(nullptr != cvpx.get<const volatile int*>());

}
