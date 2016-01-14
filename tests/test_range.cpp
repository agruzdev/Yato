#include "gtest/gtest.h"

#include <yato/range.h>
#include <vector>

#include <memory>
#include <algorithm>
#include <iostream>

TEST(Yato_Range, range)
{
	std::vector<int> vec = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
	auto vecRange = yato::range<std::vector<int>::iterator>(vec.begin(), vec.end());
	EXPECT_TRUE(vec.begin() == vecRange.begin());
	EXPECT_TRUE(vec.end() == vecRange.end());
	EXPECT_TRUE(vecRange.size() == 10);
	EXPECT_FALSE(vecRange.empty());
	EXPECT_TRUE(0 == *(vecRange.head()));
	EXPECT_TRUE(1 == *(vecRange.tail().head()));
	EXPECT_TRUE(2 == *(vecRange.tail().tail().head()));
}

TEST(Yato_Range, make_range)
{
	std::vector<int> vec(10);
	auto vecRange = yato::make_range(vec.begin(), vec.end());
	EXPECT_TRUE(vec.begin() == vecRange.begin());
	EXPECT_TRUE(vec.end() == vecRange.end());
	auto vecRangeConst = yato::make_range(vec.cbegin(), vec.cend());
	EXPECT_TRUE(vec.cbegin() == vecRangeConst.begin());
	EXPECT_TRUE(vec.cend() == vecRangeConst.end());
}

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

