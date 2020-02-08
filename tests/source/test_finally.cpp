/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2020 Alexey Gruzdev
 */

#include "gtest/gtest.h"

#include <yato/finally.h>


TEST(Yato_Finally, common)
{
    int flag = 0;

    try {
        yato_finally([&]{ flag = 1; });
    }
    catch(...) {
    }

    ASSERT_EQ(1, flag);
}


TEST(Yato_Finally, multiple_actions)
{
    int flag1 = 0;
    int flag2 = 0;

    try {
        yato_finally([&]{ flag1 = 1; });
        yato_finally([&]{ flag2 = 1; });
    }
    catch(...) {
    }

    ASSERT_EQ(1, flag1);
    ASSERT_EQ(1, flag2);
}

namespace
{
    class TestError {};
}

TEST(Yato_Finally, exception)
{
    int flag1 = 0;

    ASSERT_THROW({
        yato_finally([&]{ flag1 = 1; });
        throw TestError{};
    },
    TestError);

    ASSERT_EQ(1, flag1);
}

namespace
{
    class FinalError {};
}

TEST(Yato_Finally, exception2)
{
    int flag1 = 0;

    ASSERT_THROW({
        yato_finally([&]{ 
            flag1 = 1;
            throw FinalError{};
        });
        throw TestError{};
    },
    TestError);

    ASSERT_EQ(1, flag1);
}
