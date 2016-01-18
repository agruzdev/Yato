#include "gtest/gtest.h"

#include <yato/types.h>
#include <yato/array_nd.h>

#include <cstring>
#include <numeric>

TEST(Yato_Array_Nd, array_nd)
{
    yato::array_nd<int, 2> array_1d;
    EXPECT_NO_THROW(array_1d[0] = 0);
    EXPECT_NO_THROW(array_1d[1] = 0);
#if YATO_DEBUG
    EXPECT_THROW(array_1d[2] = 0, yato::assertion_error);
#endif


    yato::array_nd<int, 2, 3> array_2d;
    EXPECT_NO_THROW(array_2d[1]);
#if YATO_DEBUG
    EXPECT_THROW(array_2d[3], yato::assertion_error);
#endif
    auto&& p = array_2d[1];
    EXPECT_NO_THROW(p[0] = 1);
#if YATO_DEBUG
    EXPECT_THROW(p[4] = 0, yato::assertion_error);
#endif

    yato::array_nd<int, 2, 3, 4> array_3d;
    EXPECT_NO_THROW(array_3d[1][1][1] = 2);
    EXPECT_NO_THROW(array_3d[1][2][3] = 2);
#if YATO_DEBUG
    EXPECT_THROW(array_3d[2][1][1] = 0, yato::assertion_error);
    EXPECT_THROW(array_3d[1][3][1] = 0, yato::assertion_error);
    EXPECT_THROW(array_3d[1][1][4] = 0, yato::assertion_error);
#endif
};

TEST(Yato_Array_Nd, array_nd_2)
{
    int native_2x3[2][3] = { {1, 1, 1}, {2, 2, 2} };
    int native_3x2[3][2] = { {1, 1}, {2, 2}, {3, 3} };

    yato::array_nd<int, 2, 3> array_2x3;
    array_2x3[0][0] = 1; array_2x3[0][1] = 1; array_2x3[0][2] = 1;
    array_2x3[1][0] = 2; array_2x3[1][1] = 2; array_2x3[1][2] = 2;

    yato::array_nd<int, 3, 2> array_3x2;
    array_3x2[0][0] = 1; array_3x2[0][1] = 1;
    array_3x2[1][0] = 2; array_3x2[1][1] = 2;
    array_3x2[2][0] = 3; array_3x2[2][1] = 3;

    EXPECT_TRUE(sizeof(native_2x3) == sizeof(array_2x3));
    EXPECT_TRUE(sizeof(native_3x2) == sizeof(array_3x2));

    EXPECT_TRUE(0 == memcmp(&native_2x3[0][0], &array_2x3[0][0], sizeof(native_2x3)));
    EXPECT_TRUE(0 == memcmp(&native_3x2[0][0], &array_3x2[0][0], sizeof(native_3x2)));
};


TEST(Yato_Array_Nd, array_nd_on_heap)
{
    yato::vector_nd<float, 2> vec_2;
    EXPECT_NO_THROW(vec_2[0] = 1);
    EXPECT_NO_THROW(vec_2[1] = 2);
#if YATO_DEBUG
    EXPECT_THROW(vec_2[2] = 3, yato::assertion_error);
#endif

    yato::vector_nd<int, 2, 3> vec_2x3;
    EXPECT_NO_THROW(vec_2x3[0][2] = 1);
    EXPECT_NO_THROW(vec_2x3[1][0] = 2);
#if YATO_DEBUG
    EXPECT_THROW(vec_2x3[1][3] = 3, yato::assertion_error);
#endif
};

TEST(Yato_Array_Nd, array_nd_bool)
{
    yato::array_nd<bool, 16> arr;
    arr[3] = true;

    yato::vector_nd<bool, 16> vec;
    vec[4] = true;

    EXPECT_TRUE(true);
};

TEST(Yato_Array_Nd, array_nd_iterator)
{
    yato::array_nd<int, 4, 4> arr;
    std::iota(arr.begin(), arr.end(), 0);

    int i = 0;
    for (int x : arr) {
        EXPECT_TRUE(x == i++);
    }
};

TEST(Yato_Array_Nd, array_nd_iterator_2)
{
    int gt[2][4] = { {1, 1, 1, 1}, {2, 2, 2, 2} };
    yato::array_nd<int, 2, 4> arr;
    
    auto && p1 = arr[0];
    for (int & x : p1) {
        x = 1;
    }
    auto && p2 = arr[1];
    for (int & x : p2) {
        x = 2;
    }

    EXPECT_TRUE(0 == memcmp(&gt[0][0], &arr[0][0], sizeof(gt)));
};


TEST(Yato_Array_Nd, array_nd_copy)
{
    constexpr size_t N = 4;
    yato::array_nd<uint8_t, N, N, N> gt;
    yato::array_nd<uint8_t, N, N, N> arr;
   
    for (auto gtIt = gt.begin(), arrIt = arr.begin(); gtIt != gt.end(); ++gtIt, ++arrIt) {
        uint8_t val = yato::narrow_cast<uint8_t>(std::rand() % 256);
        *gtIt = val;
        *arrIt = val;
    }

    auto copy{ arr };
    for (auto copyIt = copy.begin(), arrIt = arr.begin(); copyIt != copy.end(); ++copyIt, ++arrIt) {
        EXPECT_TRUE(*copyIt == *arrIt);
    }

    for (int i = 0; i < 10; ++i) {
        copy[std::rand() % N][std::rand() % N][std::rand() % N] = yato::narrow_cast<uint8_t>(std::rand() % 256);
    }
    for (auto gtIt = gt.begin(), arrIt = arr.begin(); gtIt != gt.end(); ++gtIt, ++arrIt) {
        EXPECT_TRUE(*gtIt == *arrIt);
    }

    arr = copy;
    for (auto copyIt = copy.begin(), arrIt = arr.begin(); copyIt != copy.end(); ++copyIt, ++arrIt) {
        EXPECT_TRUE(*copyIt == *arrIt);
    }
};

