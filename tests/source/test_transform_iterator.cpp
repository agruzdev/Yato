/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2019 Alexey Gruzdev
 */

#include "gtest/gtest.h"

#include <iostream>
#include <list>
#include <forward_list>
#include <numeric>
#include <yato/transform_iterator.h>
#include <yato/zip_iterator.h>

TEST(Yato_TransformIterator, base)
{
    using input_iterator = std::istream_iterator<int>;
    using output_iterator = std::ostream_iterator<int>;
    using forward_iterator = std::forward_list<int>::iterator;
    using bidirectional_iterator = std::list<int>::iterator;
    using random_access_iterator = int*;

    static_assert(std::is_same<yato::transform_iterator<std::function<int(int)>, input_iterator>::iterator_category, std::input_iterator_tag>::value, "zip_iterator trait failed");
    static_assert(std::is_same<yato::transform_iterator<std::function<int(output_iterator&)>, output_iterator>::iterator_category, std::output_iterator_tag>::value, "zip_iterator trait failed");
    static_assert(std::is_same<yato::transform_iterator<std::function<int(int)>, forward_iterator>::iterator_category, std::forward_iterator_tag>::value, "zip_iterator trait failed");
    static_assert(std::is_same<yato::transform_iterator<std::function<int(int)>, bidirectional_iterator>::iterator_category, std::bidirectional_iterator_tag>::value, "zip_iterator trait failed");
    static_assert(std::is_same<yato::transform_iterator<std::function<int(int)>, random_access_iterator>::iterator_category, std::random_access_iterator_tag>::value, "zip_iterator trait failed");
}

TEST(Yato_TransformIterator, increment)
{
    std::vector<int> a = { 0, 1, 2, 3, 4 };
    
    auto increment = [](const int & x) {return x + 1; };
    yato::transform_iterator<std::function<int(const int&)>, std::vector<int>::iterator> it(a.begin(), increment);
    
    for (int i = 0; i < 5; ++i, ++it) {
        EXPECT_EQ(i + 1, *it);
    }
}

TEST(Yato_TransformIterator, decrement)
{
    std::vector<int> a = { 0, 1, 2, 3, 4 };
    auto increment = [](const int & x) {return x + 1; };

    yato::transform_iterator<std::function<int(const int&)>, std::vector<int>::iterator> it(std::prev(a.end()), increment);
#ifndef YATO_MSVC_2013
    for (int i = 5; i > 1; --i, --it) {
#else
    // In MSVC 2013 templated operator-- leads to crash due to a bug 
    for (int i = 5; i > 1; --i, it.operator--()) {
#endif
        EXPECT_EQ(i, *it);
    }
}

TEST(Yato_TransformIterator, loop)
{
    std::vector<int> a = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    
    auto erase = [](int & x) { int tmp = x; x = 0; return tmp; };

    using iter_type = yato::transform_iterator<std::function<int(int&)>, std::vector<int>::iterator>;
    auto iter = iter_type(a.begin(), erase);
    auto end = iter_type(a.end(), erase);
    for (int i = 1; iter != end; ++iter, ++i) {
        EXPECT_EQ(i, *iter);
    }
    for (int x : a) {
        EXPECT_EQ(0, x);
    }
}

namespace
{
    struct SomeOp
    {
        float operator()(int && x) const {
            return x + 1.0f;
        }
    };

    static int increment_foo(int & x)
    {
        return x + 1;
    }
}

TEST(Yato_TransformIterator, make_transform_iterator)
{
    std::vector<int> a = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    const std::vector<int> b = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };

    auto increment = [](const int & x) {return x + 1; };
    auto fun = std::function<int(const int&)>(increment);

    auto iter1 = yato::make_transform_iterator(b.begin(), increment);
    auto iter2 = yato::make_transform_iterator(b.begin(), fun);
    auto iter3 = yato::make_transform_iterator(a.begin(), &increment_foo);
    auto iter4 = yato::make_transform_iterator<SomeOp>(std::make_move_iterator(a.begin()));
    
    auto iter5 = yato::make_transform_iterator(b.begin(), increment);
    iter1 = iter5;
    
    auto void_foo = [](int) -> void {};

    auto iter6 = yato::make_transform_iterator(a.cbegin(), void_foo);
    auto iter7 = yato::make_transform_iterator(a.begin(), void_foo);
    iter6 = iter7;
}

TEST(Yato_TransformIterator, zip_transform)
{
    const std::vector<int> a = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    const std::vector<long> b = { 11, 12, 13, 14, 15, 16, 17, 18, 19 };
    
    auto multiply = [](const std::tuple<int, long> & x) {
        return std::get<0>(x) * std::get<1>(x);
    };
    
    auto trIt  = yato::make_transform_iterator(yato::make_zip_iterator(a.begin(), b.begin()), multiply);
    auto trEnd = yato::make_transform_iterator(yato::make_zip_iterator(a.end(), b.end()), multiply);
    
    float sum = std::accumulate(trIt, trEnd, 0.0f);
    
    EXPECT_EQ(735, sum);
}


