#include "gtest/gtest.h"

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
    for (int i = 5; i > 1; --i, --zipIt) {
        EXPECT_EQ(i - 1, std::get<0>(*zipIt));
        EXPECT_EQ(static_cast<float>(i), std::get<1>(*zipIt));
    }
}

TEST(Yato_ZipIterator, common)
{
    //std::vector<int> a = { 1, 2, 4, 5, 6, 7, 8, 9 };
    //std::vector<float> b = { 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 3.0f };
    //
    //std::for_each(
    //    boost::make_zip_iterator(boost::make_tuple(a.begin(), b.cbegin())),
    //    boost::make_zip_iterator(boost::make_tuple(a.end(), b.cend())),
    //    [](const boost::tuple<const int&, const float&> & t) {
    //    std::cout << t.get<0>() * t.get<1>() << std::endl;
    //}
    //);
}
