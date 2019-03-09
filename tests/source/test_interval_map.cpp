/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2019 Alexey Gruzdev
 */

#include "gtest/gtest.h"

#include <array>
#include <cstdlib>

#include <yato/interval_map.h>

TEST(Yato_IntervalMap, common1)
{
    yato::interval_map<size_t, char> myMap('a');
    EXPECT_EQ('a', myMap[1]);
}

TEST(Yato_IntervalMap, common2)
{
    yato::interval_map<int, char> myMap('A');
    myMap.assign(0, 100, 'B');

    EXPECT_EQ('A', myMap[-1]);
    EXPECT_EQ('B', myMap[0]);
    EXPECT_EQ('B', myMap[2]);
    EXPECT_EQ('A', myMap[100]);
    EXPECT_EQ('A', myMap[101]);
}

TEST(Yato_IntervalMap, common3)
{
    yato::interval_map<int, char> myMap('A');
    myMap.assign(0, 100, 'B');
    myMap.assign(10, 50, 'C');
    myMap.assign(40, 120, 'D');

    EXPECT_EQ('B', myMap[0]);
    EXPECT_EQ('B', myMap[9]);
    EXPECT_EQ('C',  myMap[10]);
    EXPECT_EQ('C', myMap[11]);
    EXPECT_EQ('D', myMap[41]);
    EXPECT_EQ('D', myMap[50]);
    EXPECT_EQ('D', myMap[100]);
    EXPECT_EQ('A', myMap[120]);
    EXPECT_EQ('A', myMap[121]);
}

TEST(Yato_IntervalMap, common4)
{
    yato::interval_map<int, char> myMap('A');
    myMap.assign(-2, 0, 'B');
    myMap.assign(23, 23, 'B');

    EXPECT_EQ('A', myMap[0]);
    EXPECT_EQ('A', myMap[10]);
    EXPECT_EQ('A', myMap[100]);
}

TEST(Yato_IntervalMap, common5)
{
    yato::interval_map<unsigned short, char> myMap('A');
    myMap.assign(10, 20, 'A');

    EXPECT_EQ('A', myMap[0]);
    EXPECT_EQ('A', myMap[10]);
    EXPECT_EQ('A', myMap[100]);
}

TEST(Yato_IntervalMap, common6)
{
    yato::interval_map<unsigned short, char> myMap('A');
    myMap.assign( std::numeric_limits<unsigned short>::min(), std::numeric_limits<unsigned short>::max(), 'B');

    EXPECT_EQ('B', myMap[0]);
    EXPECT_EQ('B', myMap[10]);
    EXPECT_EQ('B', myMap[100]);
    EXPECT_EQ('A', myMap[std::numeric_limits<unsigned short>::max()]);
}

TEST(Yato_IntervalMap, common7)
{
    yato::interval_map<unsigned short, char> myMap('A');
    myMap.assign(10, 20, 'A');

    EXPECT_EQ('A', myMap[0]);
    EXPECT_EQ('A', myMap[10]);
    EXPECT_EQ('A', myMap[100]);
}

TEST(Yato_IntervalMap, common8)
{
    yato::interval_map<unsigned short, char> myMap('A');
    myMap.assign(0, 10, 'C');
    myMap.assign(20, 40, 'B');
    myMap.assign(10, 20, 'C');

    EXPECT_EQ('C', myMap[0]);
    EXPECT_EQ('C', myMap[9]);
    EXPECT_EQ('C', myMap[10]);
    EXPECT_EQ('B', myMap[20]);
    EXPECT_EQ('B', myMap[39]);
    EXPECT_EQ('A', myMap[100]);
}

TEST(Yato_IntervalMap, common9)
{
    yato::interval_map<unsigned short, char> myMap('A');
    myMap.assign(10, 40, 'B');
    myMap.assign(15, 35, 'B');

    EXPECT_EQ('A', myMap[0]);
    EXPECT_EQ('A', myMap[9]);
    EXPECT_EQ('B', myMap[10]);
    EXPECT_EQ('B', myMap[20]);
    EXPECT_EQ('B', myMap[39]);
    EXPECT_EQ('A', myMap[100]);
}

TEST(Yato_IntervalMap, bruteforce)
{
    const size_t N = 100000;
    for (int t = 0; t < 100; ++t) {
        //random test set
        std::array<size_t[3], 100> test;
        for (size_t i = 0; i < test.size(); ++i) {
            size_t a = rand() % N;
            size_t b = rand() % N;
            size_t v = rand() % 10;
            test[i][0] = std::min(a, b);
            test[i][1] = std::max(a, b);
            test[i][2] = v;
        }
        //histogram solution
        size_t hist[N] = {0};
        for (size_t i = 0; i < test.size(); ++i) {
            size_t l = test[i][0];
            size_t r = test[i][1];
            size_t v = test[i][2];
            if (!(l < r)) {
                continue;
            }
            for (size_t k = l; k < r; ++k) {
                hist[k] = v;
            }
        }
        //map solution
        yato::interval_map<size_t, size_t> myMap(0);
        for (size_t i = 0; i < test.size(); ++i) {
            size_t l = test[i][0];
            size_t r = test[i][1];
            size_t v = test[i][2];
            myMap.assign(l, r, v);
        }
        //compare
        bool pass = true;
        for (size_t i = 0; i < N; ++i) {
            size_t h = hist[i];
            size_t m = myMap[i];
            if (h != m) {
                pass = false;
                break;
            }
        }
        EXPECT_TRUE(pass);
    }
}
