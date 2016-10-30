#include "gtest/gtest.h"

#include <memory>

#define YATO_THROW_ON_ASSERT
#include <yato/array_view.h>

TEST(Yato_ArrayView, common)
{
    int arr[60];
    std::iota(std::begin(arr), std::end(arr), 1);

    yato::array_view_3d<int> view(arr, yato::dims(2, 3, 4), yato::dims(5, 6));

    EXPECT_EQ(2, view.size(0));
    EXPECT_EQ(3, view.size(1));
    EXPECT_EQ(4, view.size(2));

    EXPECT_EQ(5, view.stride(0));
    EXPECT_EQ(6, view.stride(1));

    EXPECT_EQ(24, view.total_size());
    EXPECT_EQ(60, view.total_reserved());

    auto r = view.dimensions_range();
    EXPECT_EQ(2, *(r.begin()));
    EXPECT_EQ(3, *(r.begin() + 1));
    EXPECT_EQ(4, *(r.begin() + 2));

    auto dims = view.dimensions();
    EXPECT_EQ(2, dims[0]);
    EXPECT_EQ(3, dims[1]);
    EXPECT_EQ(4, dims[2]);

    int i = 0;
    for (auto it = view.plain_begin(); it != view.plain_end(); ++it, ++i) {
        EXPECT_EQ(arr[i], *it);
    }
    
}

TEST(Yato_ArrayView, common_2)
{
    int arr[3][3] = { {1, 2, 3}, {4, 5, 6}, {7, 8, 9} };
    yato::array_view_2d<int> view(&arr[0][0], yato::dims(2, 2), yato::dims(3));
    EXPECT_EQ(1, view[0][0]);
    EXPECT_EQ(2, view[0][1]);
    EXPECT_EQ(4, view[1][0]);
    EXPECT_EQ(5, view[1][1]);
}

TEST(Yato_ArrayView, array_view)
{
    int arr[10] = { 0 };

    yato::array_view<int> view(arr, yato::dims(10));
    EXPECT_TRUE(view.size(0) == 10);
    for (int i = 0; i < 10; ++i) {
        view[i] = i + 1;
        EXPECT_TRUE(arr[i] == i + 1);
    }
    EXPECT_THROW(view.at(10), yato::out_of_range_error);

#if defined(YATO_DEBUG) && (YATO_DEBUG != 0)
    int* p = nullptr;
    EXPECT_THROW(yato::array_view<int>(p, yato::dims(100)), yato::assertion_error);
#endif

    int j = 1;
    for (auto x : view.plain_crange()){
        EXPECT_TRUE(x == j++);
    }
}

TEST(Yato_ArrayView, array_view_const)
{
    const int arr[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

    yato::array_view<const int> view(arr, yato::dims(10));
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

    const std::array<int, 4> arr_4 = { { 0 } };
    auto view_4 = yato::make_view(arr_4);
    YATO_MAYBE_UNUSED(view_4);

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

    EXPECT_THROW(view.at(2, 0), yato::out_of_range_error);
    EXPECT_THROW(view.at(0, 3), yato::out_of_range_error);
    EXPECT_THROW(view.at(1, 3), yato::out_of_range_error);
}

TEST(Yato_ArrayView, array_view_nd_2)
{
    static const size_t Size_1 = 10;
    yato::uint8_t arr_1d[Size_1];

    auto view = yato::make_view(arr_1d);

    for (size_t i = 0; i < Size_1; ++i) {
        yato::uint8_t x = static_cast<yato::uint8_t>(std::rand() % 256);
        arr_1d[i] = x;
        view[i] = x;
    }

    const auto const_view = yato::make_view(arr_1d);
    for (size_t i = 0; i < Size_1; ++i) {
        EXPECT_TRUE(arr_1d[i] == view[i]);
        EXPECT_TRUE(const_view[i] == view[i]);
    }
}

TEST(Yato_ArrayView, array_view_nd_3)
{
    static const size_t Size_1 = 10;
    static const size_t Size_2 = 20;
    yato::uint8_t arr_2d[Size_1][Size_2];

    auto view = yato::make_view(arr_2d);

    for (size_t i = 0; i < Size_1; ++i) {
        for (size_t j = 0; j < Size_2; ++j) {
            yato::uint8_t x = static_cast<yato::uint8_t>(std::rand() % 256);
            arr_2d[i][j] = x;
            view[i][j] = x;
        }
    }

    const auto const_view = yato::make_view(arr_2d);
    for (size_t i = 0; i < Size_1; ++i) {
        for (size_t j = 0; j < Size_2; ++j) {
            EXPECT_TRUE(arr_2d[i][j] == view[i][j]);
            EXPECT_TRUE(const_view[i][j] == view[i][j]);
        }
    }
}

TEST(Yato_ArrayView, array_view_nd_4)
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
                EXPECT_TRUE(const_view[i][j][k] == view[i][j][k]);
            }
        }
    }
}

TEST(Yato_ArrayView, array_view_nd_5)
{
    static const size_t Size_1 = 10;
    yato::uint8_t arr_1d[Size_1];

    auto view = yato::make_view(arr_1d);

    for (size_t i = 0; i < Size_1; ++i) {
        yato::uint8_t x = static_cast<yato::uint8_t>(std::rand() % 256);
        arr_1d[i] = x;
        view.at(i) = x;
    }

    const auto const_view = yato::make_view(arr_1d);
    for (size_t i = 0; i < Size_1; ++i) {
        EXPECT_TRUE(arr_1d[i] == view.at(i));
        EXPECT_TRUE(const_view[i] == view.at(i));
    }
}

TEST(Yato_ArrayView, array_view_nd_6)
{
    static const size_t Size_1 = 10;
    static const size_t Size_2 = 20;
    yato::uint8_t arr_2d[Size_1][Size_2];

    auto view = yato::make_view(arr_2d);

    for (size_t i = 0; i < Size_1; ++i) {
        for (size_t j = 0; j < Size_2; ++j) {
            yato::uint8_t x = static_cast<yato::uint8_t>(std::rand() % 256);
            arr_2d[i][j] = x;
            view.at(i, j) = x;
        }
    }

    const auto const_view = yato::make_view(arr_2d);
    for (size_t i = 0; i < Size_1; ++i) {
        for (size_t j = 0; j < Size_2; ++j) {
            EXPECT_TRUE(arr_2d[i][j] == view.at(i, j));
            EXPECT_TRUE(const_view[i][j] == view.at(i, j));
        }
    }
}


TEST(Yato_ArrayView, array_view_nd_7)
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
                EXPECT_TRUE(const_view[i][j][k] == view.at(i, j, k));
            }
        }
    }
}


TEST(Yato_ArrayView, array_view_nd_8)
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


TEST(Yato_ArrayView, reshape)
{
    int raw[6] = { 1, 2, 3, 4, 5, 6 };

    auto dim1 = yato::dims(6);
    EXPECT_EQ(6U, dim1.total_size());

    auto plain_view = yato::make_view(raw);
    EXPECT_EQ(6U, plain_view.size(0));
    EXPECT_EQ(6U, plain_view.total_size());

    auto view_2x3 = plain_view.reshape(yato::dims(2, 3));
    EXPECT_EQ(2U, view_2x3.size(0));
    EXPECT_EQ(3U, view_2x3.size(1));
    EXPECT_EQ(6U, view_2x3.total_size());
    EXPECT_EQ(1, view_2x3[0][0]);
    EXPECT_EQ(2, view_2x3[0][1]);
    EXPECT_EQ(3, view_2x3[0][2]);
    EXPECT_EQ(4, view_2x3[1][0]);
    EXPECT_EQ(5, view_2x3[1][1]);
    EXPECT_EQ(6, view_2x3[1][2]);

    auto view_3x2 = view_2x3.reshape(yato::dims(3, 2));
    EXPECT_EQ(3U, view_3x2.size(0));
    EXPECT_EQ(2U, view_3x2.size(1));
    EXPECT_EQ(6U, view_3x2.total_size());
    EXPECT_EQ(1, view_3x2[0][0]);
    EXPECT_EQ(2, view_3x2[0][1]);
    EXPECT_EQ(3, view_3x2[1][0]);
    EXPECT_EQ(4, view_3x2[1][1]);
    EXPECT_EQ(5, view_3x2[2][0]);
    EXPECT_EQ(6, view_3x2[2][1]);

    auto view_6 = view_3x2.reshape(yato::dims(6));
    EXPECT_EQ(6U, view_6.size(0));
    EXPECT_EQ(6U, view_6.total_size());
    EXPECT_EQ(1, view_6[0]);
    EXPECT_EQ(2, view_6[1]);
    EXPECT_EQ(3, view_6[2]);
    EXPECT_EQ(4, view_6[3]);
    EXPECT_EQ(5, view_6[4]);
    EXPECT_EQ(6, view_6[5]);
}

TEST(Yato_ArrayView, reshape_2)
{
    int raw[6] = { 1, 2, 3, 4, 5, 6 };

    auto dim1 = yato::dims(6);
    EXPECT_EQ(6U, dim1.total_size());

    auto plain_view = yato::make_view(raw);
    EXPECT_EQ(6U, plain_view.size(0));
    EXPECT_EQ(6U, plain_view.total_size());

    auto view_2x2 = plain_view.reshape(yato::dims(2, 2), yato::dims(3));
    EXPECT_EQ(2U, view_2x2.size(0));
    EXPECT_EQ(2U, view_2x2.size(1));
    EXPECT_EQ(4U, view_2x2.total_size());
    EXPECT_EQ(1, view_2x2[0][0]);
    EXPECT_EQ(2, view_2x2[0][1]);
    EXPECT_EQ(4, view_2x2[1][0]);
    EXPECT_EQ(5, view_2x2[1][1]);

    auto view_6 = view_2x2.reshape(yato::dims(6));
    EXPECT_EQ(6U, view_6.size(0));
    EXPECT_EQ(6U, view_6.total_size());
    EXPECT_EQ(1, view_6[0]);
    EXPECT_EQ(2, view_6[1]);
    EXPECT_EQ(3, view_6[2]);
    EXPECT_EQ(4, view_6[3]);
    EXPECT_EQ(5, view_6[4]);
    EXPECT_EQ(6, view_6[5]);
}
