#include "gtest/gtest.h"

#include <iostream>
#include <list>
#include <forward_list>
#include <numeric>
#include <cctype>
#include <yato/filter_iterator.h>

TEST(Yato_FilterIterator, base)
{
    using input_iterator = std::istream_iterator<int>;
    using output_iterator = std::ostream_iterator<int>;
    using forward_iterator = std::forward_list<int>::iterator;
    using bidirectional_iterator = std::list<int>::iterator;
    using random_access_iterator = int*;

    static_assert(std::is_same<yato::filter_iterator<std::function<int(int)>, input_iterator>::iterator_category, std::input_iterator_tag>::value, "zip_iterator trait failed");
    //static_assert(std::is_same<yato::filter_iterator<std::function<int(output_iterator&)>, output_iterator>::iterator_category, std::output_iterator_tag>::value, "zip_iterator trait failed");
    static_assert(std::is_same<yato::filter_iterator<std::function<int(int)>, forward_iterator>::iterator_category, std::forward_iterator_tag>::value, "zip_iterator trait failed");
    static_assert(std::is_same<yato::filter_iterator<std::function<int(int)>, bidirectional_iterator>::iterator_category, std::bidirectional_iterator_tag>::value, "zip_iterator trait failed");
    static_assert(std::is_same<yato::filter_iterator<std::function<int(int)>, random_access_iterator>::iterator_category, std::bidirectional_iterator_tag>::value, "zip_iterator trait failed");
}

TEST(Yato_FilterIterator, create_and_assign)
{
    std::vector<int> v = { 1, 2, 3, 4, 5, 6 };

    yato::filter_iterator< std::function<bool(int)>, std::vector<int>::iterator > it1(v.begin(), v.end(), [](int) { return true; });
    yato::filter_iterator< std::function<bool(int)>, std::vector<int>::iterator > it2(v.begin(), v.end(), [](int x) { return x > 2; });
    yato::filter_iterator< std::function<bool(int)>, std::vector<int>::const_iterator > it3(v.end(), v.end(), [](int) { return false; });
    yato::filter_iterator< std::function<bool(int)>, std::vector<int>::const_iterator > it4(v.end(), v.end(), [](int x) { return x > 2; });
    
    yato::filter_iterator< std::function<bool(int)>, std::vector<int>::iterator, yato::dereference_policy_no_caching, false > it5(v.begin(), [](int) { return true; });
    yato::filter_iterator< std::function<bool(int)>, std::vector<int>::iterator, yato::dereference_policy_no_caching, false > it6(v.begin(), [](int x) { return x > 2; });
    //yato::filter_iterator< std::function<bool(int)>, std::vector<int>::iterator, false > it7(v.begin(), [](int x) { return x > 9; }); //Error!

    yato::filter_iterator< std::function<bool(int)>, std::vector<int>::iterator > it10(it1);
    yato::filter_iterator< std::function<bool(const int&)>, std::vector<int>::const_iterator > it11(it2);

    auto it21(std::move(it10));
    yato::filter_iterator< std::function<bool(const int&)>, std::vector<int>::const_iterator > it22(std::move(it2));

    using std::swap;
    swap(it1, it2);
    it22 = it21;
    it1  = std::move(it21);
    it22 = std::move(it1);
}

#ifndef YATO_MSVC_2013
TEST(Yato_FilterIterator, increment)
{
    std::vector<int> v = { -1, 1, -2, -1, 2, -2, -3, -2, 3, -4, 4 };

    yato::filter_iterator< std::function<bool(int)>, std::vector<int>::const_iterator > it(v.cbegin(), v.cend(), [](int x) { return x > 0; });
    EXPECT_EQ(1, *it);
    ++it;
    EXPECT_EQ(2, *it);
    it++;
    EXPECT_EQ(3, *it++);
    EXPECT_EQ(4, *it);
}

TEST(Yato_FilterIterator, decrement)
{
    std::vector<int> v = { -1, 1, -2, -1, 2, -2, -3, -2, 3, -4, 4, -5, -9 };

    yato::filter_iterator< std::function<bool(int)>, std::vector<int>::const_iterator > it(v.cend(), v.cend(), [](int x) { return x > 0; });

    --it;
    EXPECT_EQ(4, *it);
    --it;
    EXPECT_EQ(3, *it);
    it--;
    EXPECT_EQ(2, *it--);
    EXPECT_EQ(1, *it);
}
#endif

TEST(Yato_FilterIterator, compare)
{
    std::vector<int> v = { -1, 1, -2, -1, 2, -2, -3, -2, 3, -4, 4, -5, -9 };

    yato::filter_iterator< std::function<bool(int)>, std::vector<int>::const_iterator > it1(v.cbegin(), v.cend(), [](int x) { return x > 0; });
    yato::filter_iterator< std::function<bool(int)>, std::vector<int>::const_iterator > it2(v.cbegin(), v.cend(), [](int x) { return x > 2; });

    EXPECT_TRUE(it1 == v.begin() + 1);
    EXPECT_TRUE(it2 == v.begin() + 8);

    EXPECT_TRUE(it1 != it2);
#ifndef YATO_MSVC_2013
    std::advance(it1, 2);
#else
    ++it1;
    ++it1;
#endif
    EXPECT_TRUE(it1 == it2);
    EXPECT_TRUE(it1++ == it2++);
#ifndef YATO_MSVC_2013
    EXPECT_TRUE(it1-- == it2--);
#else
    it1.operator--();
    it2.operator--();
    EXPECT_TRUE(it1 == it2);
#endif
    EXPECT_TRUE(it1 == it2);
#ifndef YATO_MSVC_2013
    std::advance(it1, 2);
#else
    ++it1;
    ++it1;
#endif
    EXPECT_TRUE(it1 == v.cend());
}

TEST(Yato_FilterIterator, make_filter_iterator)
{
    std::vector<int> v = { -1, 1, -2, -1, 2, -2, -3, -2, 3, -4, 4, -5, -9 };
    auto lambda = [](const int & x) { return x >= 0; };
    auto it1 = yato::make_filter_iterator(v.begin(), v.end(), lambda);
    static_assert(std::is_same<decltype(it1)::predicate_type, decltype(lambda)>::value, "filter iterator fail");
}

#ifndef YATO_MSVC_2013
TEST(Yato_FilterIterator, input_iterator)
{
    std::istringstream str1("0 1 0 2 0 0 3");
    auto it1 = yato::make_filter_iterator(std::istream_iterator<int>(str1), [](int x) { return x > 0; });
    EXPECT_EQ(1, *it1++);
    EXPECT_EQ(2, *it1++);
    EXPECT_EQ(3, *it1);

    std::istringstream str2("1 2 1 3 1 1 4");
    auto p = [](int x) { return x > 1; };
    int s = std::accumulate(yato::make_filter_iterator(std::istream_iterator<int>(str2), std::istream_iterator<int>(), p), yato::make_filter_iterator(std::istream_iterator<int>(), std::istream_iterator<int>(), p), 0);
    EXPECT_EQ(9, s);
}
#endif

