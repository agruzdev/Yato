#include "gtest/gtest.h"

#include <yato/range.h>
#include <yato/types.h>
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

TEST(Yato_Range, numeric_range_1)
{
	int i = 0;
	for (int x : yato::make_range(0, 100)) {
		EXPECT_TRUE(x == i++);
	}
}

TEST(Yato_Range, numeric_range_2)
{
	size_t i = 0;
	for (size_t x : yato::make_range(100U)) {
		EXPECT_TRUE(x == i++);
	}
}

TEST(Yato_Range, numeric_range_3)
{
	using namespace yato::literals;
	yato::uint8_t i = 0;
	for (auto x : yato::make_range(100_u8)) {
		EXPECT_TRUE(x == i++);
	}
}
