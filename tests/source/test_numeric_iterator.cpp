/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2019 Alexey Gruzdev
 */

#include "gtest/gtest.h"

#include <yato/numeric_iterator.h>

#include <memory>
#include <algorithm>
#include <iostream>

TEST(Yato_Range, numeric_iterator_1)
{
    auto begin = yato::numeric_iterator<int>(0);
    auto end = yato::numeric_iterator<int>(10);

    for (int i = 0; begin != end; ++begin, ++i) {
        EXPECT_TRUE(i == *begin);
    }
}

TEST(Yato_Range, numeric_iterator_2)
{
    auto begin = yato::numeric_iterator<int>(0);
    auto end = yato::numeric_iterator<int>(5);

    std::vector<int> vec(5, -1);
    std::copy(begin, end, vec.begin());
}
