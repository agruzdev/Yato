#include "gtest/gtest.h"

#include <memory>
#include <yato/array_view.h>



TEST(Yato_Array_View, array_view)
{
    int arr[10] = { 0 };

    yato::array_view<int> view(arr, 10);
    EXPECT_TRUE(view.size() == 10);
    for (int i = 0; i < 10; ++i) {
        view[i] = i + 1;
        EXPECT_TRUE(arr[i] == i + 1);
    }
    EXPECT_THROW(view.at(10), yato::assertion_error);

    int* p = nullptr;
    EXPECT_THROW(yato::array_view<int>(p, 100), yato::assertion_error);

}

TEST(Yato_Array_View, array_view_const)
{
    const int arr[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

    yato::array_view<const int> view(arr, 10);
    EXPECT_TRUE(view.size() == 10);
    for (int i = 0; i < 10; ++i) {
        EXPECT_TRUE(view[i] == i);
    }

}


TEST(Yato_Array_View, make_view)
{
    int arr_1[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };;
    auto view_1 = yato::make_view(arr_1);
    EXPECT_TRUE(view_1.size() == 10);
    for (int i = 0; i < 10; ++i) {
        EXPECT_TRUE(view_1[i] == i);
    }

    const float arr_2[10] = { 0 };
    auto view_2 = yato::make_view(arr_2);
    EXPECT_TRUE(view_2.size() == 10);


    std::array<int, 4> arr_3;
    auto view_3 = yato::make_view(arr_3);
    EXPECT_TRUE(view_3.size() == 4);
    view_3[0] = 0;

    const std::array<int, 4> arr_4 = { 0 };
    auto view_4 = yato::make_view(arr_4);

    std::vector<int> arr_5(5, 0);
    auto view_5 = yato::make_view(arr_5);
    EXPECT_TRUE(view_5.size() == 5);
    view_5[0] = 10;

    const std::vector<int> arr_6(5, 0);
    auto view_6 = yato::make_view(arr_6);
    EXPECT_TRUE(view_6.size() == 5);
}

TEST(Yato_Array_View, array_view_range)
{
    uint8_t arr[10] = { 0 };
    for (uint8_t & x : yato::make_view(arr)) {
        x = 1;
    }
    for (int i = 0; i < 10; ++i) {
        EXPECT_TRUE(arr[i] == 1);
    }

}



TEST(Yato_Array_View, array_view_nd)
{
    int arr_2d[2][3] = { {1, 1, 1}, {2, 2, 2} };
    auto view = yato::make_view(arr_2d);
    
    EXPECT_TRUE(view.size() == 6);
    EXPECT_TRUE(view[0].size() == 3);

    EXPECT_TRUE(view[0][0] == 1);
    EXPECT_TRUE(view[0][1] == 1);
    EXPECT_TRUE(view[0][2] == 1);

    EXPECT_TRUE(view[1][0] == 2);
    EXPECT_TRUE(view[1][1] == 2);
    EXPECT_TRUE(view[1][2] == 2);

}

TEST(Yato_Array_View, array_view_nd_1)
{
    static const size_t Size_1 = 10;
    static const size_t Size_2 = 20;
    static const size_t Size_3 = 15;
    yato::uint8_t arr_3d[Size_1][Size_2][Size_3];

    auto view = yato::make_view(arr_3d);

    for (size_t i = 0; i < Size_1; ++i) {
        for (size_t j = 0; j < Size_2; ++j) {
            for (size_t k = 0; k < Size_3; ++k) {
                yato::uint8_t x = static_cast<yato::uint8_t>(std::rand() % 256);
                arr_3d[i][j][k] = x;
                view[i][j][k] = x;
            }
        }
    }

    const auto const_view = yato::make_view(arr_3d);
    for (size_t i = 0; i < Size_1; ++i) {
        for (size_t j = 0; j < Size_2; ++j) {
            for (size_t k = 0; k < Size_3; ++k) {
                EXPECT_TRUE(arr_3d[i][j][k] == view[i][j][k]);
            }
        }
    }
}