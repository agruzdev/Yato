#include "gtest/gtest.h"

#include <memory>
#include <yato/array_view.h>



TEST(Yato_ArrayView, array_view)
{
    int arr[10] = { 0 };

    yato::array_view<int> view(arr, 10);
    EXPECT_TRUE(view.size(0) == 10);
    for (int i = 0; i < 10; ++i) {
        view[i] = i + 1;
        EXPECT_TRUE(arr[i] == i + 1);
    }
    EXPECT_THROW(view.at(10), yato::assertion_error);

    int* p = nullptr;
    EXPECT_THROW(yato::array_view<int>(p, 100), yato::assertion_error);

    int j = 1;
    for (auto x : view.plain_crange()){
        EXPECT_TRUE(x == j++);
    }
}

TEST(Yato_ArrayView, array_view_const)
{
    const int arr[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

    yato::array_view<const int> view(arr, 10);
    EXPECT_TRUE(view.size() == 10);
    for (int i = 0; i < 10; ++i) {
        EXPECT_TRUE(view[i] == i);
    }

}


TEST(Yato_ArrayView, make_view)
{
    int arr_1[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
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

TEST(Yato_ArrayView, array_view_range)
{
    uint8_t arr[10] = { 0 };
    for (uint8_t & x : yato::make_view(arr)) {
        x = 1;
    }
    for (int i = 0; i < 10; ++i) {
        EXPECT_TRUE(arr[i] == 1);
    }

}



TEST(Yato_ArrayView, array_view_nd)
{
    int arr_2d[2][3] = { {1, 1, 1}, {2, 2, 2} };
    auto view = yato::make_view(arr_2d);
    
    EXPECT_TRUE(view.total_size() == 6);
    EXPECT_TRUE(view[0].total_size() == 3);
    EXPECT_TRUE(view[1].total_size() == 3);

    EXPECT_TRUE(view[0][0] == 1);
    EXPECT_TRUE(view[0][1] == 1);
    EXPECT_TRUE(view[0][2] == 1);

    EXPECT_TRUE(view[1][0] == 2);
    EXPECT_TRUE(view[1][1] == 2);
    EXPECT_TRUE(view[1][2] == 2);

#if YATO_DEBUG
    EXPECT_THROW(view[2], yato::assertion_error);
    EXPECT_THROW(view[0][3], yato::assertion_error);
    EXPECT_THROW(view[1][3], yato::assertion_error);
#endif
}

TEST(Yato_ArrayView, array_view_nd_1)
{
    int arr_2d[2][3] = { { 1, 1, 1 },{ 2, 2, 2 } };
    auto view = yato::make_view(arr_2d);

    EXPECT_TRUE(view.total_size() == 6);
    EXPECT_TRUE(view[0].total_size() == 3);
    EXPECT_TRUE(view[1].total_size() == 3);

    EXPECT_TRUE(view.at(0, 0) == 1);
    EXPECT_TRUE(view.at(0, 1) == 1);
    EXPECT_TRUE(view.at(0, 2) == 1);

    EXPECT_TRUE(view.at(1, 0) == 2);
    EXPECT_TRUE(view.at(1, 1) == 2);
    EXPECT_TRUE(view.at(1, 2) == 2);

    EXPECT_THROW(view.at(2, 0), yato::assertion_error);
    EXPECT_THROW(view.at(0, 3), yato::assertion_error);
    EXPECT_THROW(view.at(1, 3), yato::assertion_error);
}

TEST(Yato_ArrayView, array_view_nd_2)
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

TEST(Yato_ArrayView, array_view_nd_3)
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
                view.at(i, j, k) = x;
            }
        }
    }

    const auto const_view = yato::make_view(arr_3d);
    for (size_t i = 0; i < Size_1; ++i) {
        for (size_t j = 0; j < Size_2; ++j) {
            for (size_t k = 0; k < Size_3; ++k) {
                EXPECT_TRUE(arr_3d[i][j][k] == view.at(i, j, k));
            }
        }
    }
}

TEST(Yato_ArrayView, array_view_nd_4)
{
    int arr[2][2];

    auto view = yato::make_view(arr);

    std::fill(view.plain_begin(), view.plain_end(), 1);
    EXPECT_TRUE(arr[0][0] == 1);
    EXPECT_TRUE(arr[0][1] == 1);
    EXPECT_TRUE(arr[1][0] == 1);
    EXPECT_TRUE(arr[1][1] == 1);


    int arr_3d_1[3][2][2] = {
        { { 1, 1 },{ 1, 1 } },
        { { 0, 0 },{ 0, 0 } },
        { { 3, 3 },{ 3, 3 } }
    };
    const int arr_3d_2[3][2][2] = {
        { { 1, 1 },{ 1, 1 } },
        { { 2, 2 },{ 2, 2 } },
        { { 3, 3 },{ 3, 3 } }
    };
    auto view_3d_1 = yato::make_view(arr_3d_1);
    auto view_3d_2 = yato::make_view(arr_3d_2);

    for (auto & x : view_3d_1[1].plain_range()) {
        x = 2;
    }
    
    auto it1  = view_3d_1.plain_begin();
    auto it2  = view_3d_2.plain_cbegin();
    auto end2 = view_3d_2.plain_cend();
    while(it2 != end2) {
        EXPECT_TRUE(*it1++ == *it2++);
    }
}
