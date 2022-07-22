/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2020 Alexey Gruzdev
 */

#include "gtest/gtest.h"

#include <algorithm>
#include <iostream>
#include <list>
#include <forward_list>
#include <yato/zip_iterator.h>

TEST(Yato_ZipIterator, base)
{
    using input_iterator = std::istream_iterator<int>;
    using output_iterator = std::ostream_iterator<int>;
    using forward_iterator = std::forward_list<int>::iterator;
    using bidirectional_iterator = std::list<int>::iterator;
    using random_access_iter = int*;

    static_assert(std::is_same<yato::zip_iterator<input_iterator, input_iterator>::iterator_category, std::input_iterator_tag>::value, "zip_iterator trait failed");
    static_assert(std::is_same<yato::zip_iterator<output_iterator, output_iterator>::iterator_category, std::output_iterator_tag>::value, "zip_iterator trait failed");
    static_assert(std::is_same<yato::zip_iterator<forward_iterator, forward_iterator>::iterator_category, std::forward_iterator_tag>::value, "zip_iterator trait failed");
    static_assert(std::is_same<yato::zip_iterator<bidirectional_iterator, bidirectional_iterator>::iterator_category, std::bidirectional_iterator_tag>::value, "zip_iterator trait failed");
    static_assert(std::is_same<yato::zip_iterator<random_access_iter, random_access_iter>::iterator_category, std::random_access_iterator_tag>::value, "zip_iterator trait failed");

    static_assert(std::is_same<yato::zip_iterator<input_iterator, forward_iterator, bidirectional_iterator, random_access_iter>::iterator_category, std::input_iterator_tag>::value, "zip_iterator trait failed");
    static_assert(std::is_same<yato::zip_iterator<forward_iterator, bidirectional_iterator, random_access_iter>::iterator_category, std::forward_iterator_tag>::value, "zip_iterator trait failed");
    static_assert(std::is_same<yato::zip_iterator<bidirectional_iterator, random_access_iter>::iterator_category, std::bidirectional_iterator_tag>::value, "zip_iterator trait failed");
}

namespace
{
    template <typename _T>
    struct increments
    {
        YATO_CONSTEXPR_FUNC
        _T operator()(const _T & x) const
        {
            return x + 1;
        }
    };
}

TEST(Yato_ZipIterator, increment)
{
    std::vector<int> a = { 0, 1, 2, 3, 4 };
    std::vector<float> b = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f };
    
    yato::zip_iterator<std::vector<int>::iterator, std::vector<float>::iterator> zipIt(std::make_tuple(a.begin(), b.begin()));
    for (int i = 0; i < 5; ++i, ++zipIt) {
        EXPECT_EQ(i, std::get<0>(*zipIt));
        EXPECT_EQ(static_cast<float>(i + 1), std::get<1>(*zipIt));
    }
}

TEST(Yato_ZipIterator, decrement)
{
    std::vector<int> a = { 0, 1, 2, 3, 4 };
    std::vector<float> b = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f };

    yato::zip_iterator<std::vector<int>::iterator, std::vector<float>::iterator> zipIt(std::make_tuple(a.end() - 1, b.end() - 1));
#ifndef YATO_MSVC_2013
    for (int i = 5; i > 1; --i, --zipIt) {
#else
    // In MSVC 2013 templated operator-- leads to crash due to a bug 
    for (int i = 5; i > 1; --i, zipIt.operator--()) {
#endif
        EXPECT_EQ(i - 1, std::get<0>(*zipIt));
        EXPECT_EQ(static_cast<float>(i), std::get<1>(*zipIt));
    }
}

TEST(Yato_ZipIterator, copy)
{
    std::vector<int> a = { 1, 2, 3 };
    using iter_type = yato::zip_iterator<std::vector<int>::iterator, std::vector<int>::const_iterator>;

    auto it1 = iter_type(std::make_tuple(a.begin(), a.cbegin()));
    auto it2 = iter_type(std::make_tuple(a.end(), a.cend()));

    it1 = it2;
    auto it3(it1);

    using const_iter_type = yato::zip_iterator<std::vector<int>::const_iterator, std::vector<int>::const_iterator>;
    const_iter_type it4(it1);
    
    using std::swap;
    swap(it1, it2);

    it4 = std::move(it1);
    it4 = it2;
}

TEST(Yato_ZipIterator, loop)
{
    std::vector<int> a = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    std::vector<float> b = { 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 3.0f, 4.0f };
    float sum = 0.0f;

    using iter_type = yato::zip_iterator<std::vector<int>::iterator, std::vector<float>::const_iterator>;
    auto iter = iter_type(std::make_tuple(a.begin(), b.cbegin()));
    auto end = iter_type(std::make_tuple(a.end(), b.cend()));
    for (; iter != end; ++iter) {
        sum += std::get<0>(*iter) * std::get<1>(*iter);
    }
    EXPECT_EQ(116.0f, sum);
}

TEST(Yato_ZipIterator, make_zip_iterator)
{
    std::vector<int> a = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    std::vector<float> b = { 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 3.0f, 4.0f };

    auto iter = yato::make_zip_iterator(std::make_tuple(a.begin(), b.begin()));
    static_assert(std::is_same<decltype(iter)::iterators_tuple, std::tuple<std::vector<int>::iterator, std::vector<float>::iterator>>::value, "make_zip_iterator fail");

    auto iter2 = yato::make_zip_iterator(a.begin(), b.begin());
    static_assert(std::is_same<decltype(iter2)::iterators_tuple, std::tuple<std::vector<int>::iterator, std::vector<float>::iterator>>::value, "make_zip_iterator fail");
}

TEST(Yato_ZipIterator, loop_2)
{
    std::vector<int> a = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    std::vector<float> b = { 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 3.0f, 4.0f };
    float sum = 0.0f;
    
    std::for_each(
        yato::make_zip_iterator(std::make_tuple(a.begin(), b.cbegin())),
        yato::make_zip_iterator(std::make_tuple(a.end(), b.cend())),
        [&sum](const std::tuple<int&, const float&> & t) {
            sum += std::get<0>(t) * std::get<1>(t);
    });
    EXPECT_EQ(116.0f, sum);
}

TEST(Yato_ZipIterator, dist)
{
    std::vector<int> a = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    std::vector<float> b = { 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 3.0f, 4.0f };

    auto it1 = yato::make_zip_iterator(std::make_tuple(a.begin(), b.cbegin()));
    const auto it2 = yato::make_zip_iterator(std::make_tuple(a.end(), b.cend()));
    EXPECT_EQ(9, it2 - it1);

    ++it1;
    EXPECT_EQ(8, it2 - it1);

    it1 += 2;
    EXPECT_EQ(6, it2 - it1);

    while(it1 != it2) {
        ++it1;
    }
    EXPECT_EQ(0, it2 - it1);
}
