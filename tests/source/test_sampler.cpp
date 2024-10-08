/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2020 Alexey Gruzdev
 */

#include "gtest/gtest.h"

#include "yato/array_nd.h"
#include "yato/vector_nd.h"

TEST(Yato_Sampler, sampler_default)
{
    yato::vector_nd<int, 2> v1 = {
        { 1, 2, 5 },
        { 3, 4, 6 }
    };

    EXPECT_EQ(1, yato::load<yato::sampler_default>(v1, 0, 0));
    EXPECT_EQ(2, yato::load<yato::sampler_default>(v1, 0, 1));
    EXPECT_EQ(3, yato::load<yato::sampler_default>(v1, 1, 0));
    EXPECT_EQ(4, yato::load<yato::sampler_default>(v1, 1, 1));

    EXPECT_THROW(yato::load<yato::sampler_default>(v1, 1, 3), yato::out_of_range_error);
    EXPECT_THROW(yato::load<yato::sampler_default>(v1, 2, 1), yato::out_of_range_error);
    EXPECT_THROW(yato::load<yato::sampler_default>(v1, 10, 1), yato::out_of_range_error);
    EXPECT_THROW(yato::load<yato::sampler_default>(v1, 10, 11), yato::out_of_range_error);

    EXPECT_EQ(1, yato::at(v1, 0, 0));
    EXPECT_EQ(2, yato::at(v1, 0, 1));
    EXPECT_EQ(3, yato::at(v1, 1, 0));
    EXPECT_EQ(4, yato::at(v1, 1, 1));

    EXPECT_EQ(&v1[0][0], &yato::at(v1, 0, 0));
    EXPECT_EQ(&v1[0][1], &yato::at(v1, 0, 1));
    EXPECT_EQ(&v1[1][0], &yato::at(v1, 1, 0));
    EXPECT_EQ(&v1[1][1], &yato::at(v1, 1, 1));

    EXPECT_THROW(yato::at<yato::sampler_default>(v1, 1, 3), yato::out_of_range_error);
    EXPECT_THROW(yato::at<yato::sampler_default>(v1, 2, 1), yato::out_of_range_error);
    EXPECT_THROW(yato::at<yato::sampler_default>(v1, 10, 1), yato::out_of_range_error);
    EXPECT_THROW(yato::at<yato::sampler_default>(v1, 10, 11), yato::out_of_range_error);
}

TEST(Yato_Sampler, sampler_no_check)
{
    yato::vector_nd<int, 2> v1 = {
        { 1, 2 },
        { 3, 4 }
    };

    EXPECT_EQ(1, yato::load<yato::sampler_no_check>(v1, 0, 0));
    EXPECT_EQ(2, yato::load<yato::sampler_no_check>(v1, 0, 1));
    EXPECT_EQ(3, yato::load<yato::sampler_no_check>(v1, 1, 0));
    EXPECT_EQ(4, yato::load<yato::sampler_no_check>(v1, 1, 1));

    EXPECT_EQ(1, yato::at<yato::sampler_no_check>(v1, 0, 0));
    EXPECT_EQ(2, yato::at<yato::sampler_no_check>(v1, 0, 1));
    EXPECT_EQ(3, yato::at<yato::sampler_no_check>(v1, 1, 0));
    EXPECT_EQ(4, yato::at<yato::sampler_no_check>(v1, 1, 1));
}

TEST(Yato_Sampler, sampler_zero)
{
    yato::vector_nd<int, 2> v1 = {
        { 1, 2 },
        { 3, 4 }
    };

    EXPECT_EQ(1, yato::load<yato::sampler_zero>(v1, 0, 0));
    EXPECT_EQ(2, yato::load<yato::sampler_zero>(v1, 0, 1));
    EXPECT_EQ(3, yato::load<yato::sampler_zero>(v1, 1, 0));
    EXPECT_EQ(4, yato::load<yato::sampler_zero>(v1, 1, 1));

    EXPECT_EQ(0, yato::load<yato::sampler_zero>(v1, -1, 0));
    EXPECT_EQ(0, yato::load<yato::sampler_zero>(v1, 0, -1));
    EXPECT_EQ(0, yato::load<yato::sampler_zero>(v1, 1, 2));
    EXPECT_EQ(0, yato::load<yato::sampler_zero>(v1, 2, 1));

    EXPECT_THROW(yato::at<yato::sampler_zero>(v1, -1, 0), yato::out_of_range_error);
    EXPECT_THROW(yato::at<yato::sampler_zero>(v1, 1, 2), yato::out_of_range_error);
}

TEST(Yato_Sampler, sampler_clamp)
{
    yato::vector_nd<int, 2> v1 = {
        { 1, 2 },
        { 3, 4 }
    };

    EXPECT_EQ(1, yato::load<yato::sampler_clamp>(v1, 0, 0));
    EXPECT_EQ(2, yato::load<yato::sampler_clamp>(v1, 0, 1));
    EXPECT_EQ(3, yato::load<yato::sampler_clamp>(v1, 1, 0));
    EXPECT_EQ(4, yato::load<yato::sampler_clamp>(v1, 1, 1));

    EXPECT_EQ(1, yato::load<yato::sampler_clamp>(v1, -1, 0));
    EXPECT_EQ(1, yato::load<yato::sampler_clamp>(v1, 0, -1));
    EXPECT_EQ(4, yato::load<yato::sampler_clamp>(v1, 1, 2));
    EXPECT_EQ(4, yato::load<yato::sampler_clamp>(v1, 2, 1));


    yato::vector_nd<int, 1> v2 = {
        { 1, 2, 3 }
    };

    auto view2 = yato::cview(v2).reshape(yato::dims(1, 1, 3));

    EXPECT_EQ(1, yato::load<yato::sampler_clamp>(view2, 0, 0, 0));
    EXPECT_EQ(2, yato::load<yato::sampler_clamp>(view2, 0, 0, 1));
    EXPECT_EQ(3, yato::load<yato::sampler_clamp>(view2, 0, 0, 2));
    EXPECT_EQ(1, yato::load<yato::sampler_clamp>(view2, 1, 0, 0));
    EXPECT_EQ(2, yato::load<yato::sampler_clamp>(view2, 0, 2, 1));
    EXPECT_EQ(3, yato::load<yato::sampler_clamp>(view2, 3, 0, 2));
    EXPECT_EQ(1, yato::load<yato::sampler_clamp>(view2, 3, 2, 0));
    EXPECT_EQ(2, yato::load<yato::sampler_clamp>(view2, 1, 1, 1));
    EXPECT_EQ(3, yato::load<yato::sampler_clamp>(view2, 4, 4, 2));

    static_assert(std::is_same<typename yato::container_traits<decltype(v2)>::container_category, yato::container_continuous_tag>::value);
    static_assert(std::is_same<typename yato::container_traits<decltype(view2)>::container_category, yato::container_strided_tag>::value);
}


TEST(Yato_Sampler, sampler_c_array)
{
    int arr[3] = {1, 2, 3};

    EXPECT_EQ(1, yato::load<yato::sampler_default>(arr, 0));
    EXPECT_EQ(2, yato::load<yato::sampler_default>(arr, 1));
    EXPECT_EQ(3, yato::load<yato::sampler_default>(arr, 2));

    EXPECT_EQ(1, yato::load<yato::sampler_no_check>(arr, 0));
    EXPECT_EQ(2, yato::load<yato::sampler_no_check>(arr, 1));
    EXPECT_EQ(3, yato::load<yato::sampler_no_check>(arr, 2));

    EXPECT_EQ(1, yato::load<yato::sampler_clamp>(arr, -1));
    EXPECT_EQ(1, yato::load<yato::sampler_clamp>(arr, 0));
    EXPECT_EQ(2, yato::load<yato::sampler_clamp>(arr, 1));
    EXPECT_EQ(3, yato::load<yato::sampler_clamp>(arr, 2));
    EXPECT_EQ(3, yato::load<yato::sampler_clamp>(arr, 10));

    EXPECT_EQ(0, yato::load<yato::sampler_zero>(arr, -1));
    EXPECT_EQ(1, yato::load<yato::sampler_zero>(arr, 0));
    EXPECT_EQ(2, yato::load<yato::sampler_zero>(arr, 1));
    EXPECT_EQ(3, yato::load<yato::sampler_zero>(arr, 2));
    EXPECT_EQ(0, yato::load<yato::sampler_zero>(arr, 10));

    yato::at(arr, 0) = 10;
    yato::at(arr, 1) = 20;
    yato::at(arr, 2) = 30;

    EXPECT_EQ(10, yato::at(arr, 0));
    EXPECT_EQ(20, yato::at(arr, 1));
    EXPECT_EQ(30, yato::at(arr, 2));
}


TEST(Yato_Sampler, sampler_std_array)
{
    std::array<int, 3> arr = { 1, 2, 3 };

    EXPECT_EQ(1, yato::load<yato::sampler_default>(arr, 0));
    EXPECT_EQ(2, yato::load<yato::sampler_default>(arr, 1));
    EXPECT_EQ(3, yato::load<yato::sampler_default>(arr, 2));

    EXPECT_EQ(1, yato::load<yato::sampler_no_check>(arr, 0));
    EXPECT_EQ(2, yato::load<yato::sampler_no_check>(arr, 1));
    EXPECT_EQ(3, yato::load<yato::sampler_no_check>(arr, 2));

    EXPECT_EQ(1, yato::load<yato::sampler_clamp>(arr, -1));
    EXPECT_EQ(1, yato::load<yato::sampler_clamp>(arr, 0));
    EXPECT_EQ(2, yato::load<yato::sampler_clamp>(arr, 1));
    EXPECT_EQ(3, yato::load<yato::sampler_clamp>(arr, 2));
    EXPECT_EQ(3, yato::load<yato::sampler_clamp>(arr, 10));

    EXPECT_EQ(0, yato::load<yato::sampler_zero>(arr, -1));
    EXPECT_EQ(1, yato::load<yato::sampler_zero>(arr, 0));
    EXPECT_EQ(2, yato::load<yato::sampler_zero>(arr, 1));
    EXPECT_EQ(3, yato::load<yato::sampler_zero>(arr, 2));
    EXPECT_EQ(0, yato::load<yato::sampler_zero>(arr, 10));
}


TEST(Yato_Sampler, sampler_std_vector)
{
    std::vector<int> arr = { 1, 2, 3 };

    EXPECT_EQ(1, yato::load<yato::sampler_default>(arr, 0));
    EXPECT_EQ(2, yato::load<yato::sampler_default>(arr, 1));
    EXPECT_EQ(3, yato::load<yato::sampler_default>(arr, 2));

    EXPECT_EQ(1, yato::load<yato::sampler_no_check>(arr, 0));
    EXPECT_EQ(2, yato::load<yato::sampler_no_check>(arr, 1));
    EXPECT_EQ(3, yato::load<yato::sampler_no_check>(arr, 2));

    EXPECT_EQ(1, yato::load<yato::sampler_clamp>(arr, -1));
    EXPECT_EQ(1, yato::load<yato::sampler_clamp>(arr, 0));
    EXPECT_EQ(2, yato::load<yato::sampler_clamp>(arr, 1));
    EXPECT_EQ(3, yato::load<yato::sampler_clamp>(arr, 2));
    EXPECT_EQ(3, yato::load<yato::sampler_clamp>(arr, 10));

    EXPECT_EQ(0, yato::load<yato::sampler_zero>(arr, -1));
    EXPECT_EQ(1, yato::load<yato::sampler_zero>(arr, 0));
    EXPECT_EQ(2, yato::load<yato::sampler_zero>(arr, 1));
    EXPECT_EQ(3, yato::load<yato::sampler_zero>(arr, 2));
    EXPECT_EQ(0, yato::load<yato::sampler_zero>(arr, 10));
}


namespace
{

    struct sampler_oob_counter
        : public yato::sampler_zero
    {
        template <typename ValueType_>
        return_type<ValueType_> boundary_value()
        {
            ++count;
            return yato::sampler_zero::boundary_value<ValueType_>();
        }

        size_t count = 0;
    };

}

TEST(Yato_Sampler, custom_sampler)
{
    yato::vector_nd<int, 2> v1 = {
        { 1, 2 },
        { 3, 4 }
    };

    sampler_oob_counter sampler{};

    EXPECT_EQ(1, yato::loads(sampler, v1, 0, 0));
    EXPECT_EQ(2, yato::loads(sampler, v1, 0, 1));
    EXPECT_EQ(3, yato::loads(sampler, v1, 1, 0));
    EXPECT_EQ(4, yato::loads(sampler, v1, 1, 1));

    EXPECT_EQ(0, yato::loads(sampler, v1, -1, 0));
    EXPECT_EQ(0, yato::loads(sampler, v1, 0, -1));
    EXPECT_EQ(0, yato::loads(sampler, v1, 1, 2));
    EXPECT_EQ(0, yato::loads(sampler, v1, 2, 1));

    EXPECT_EQ(4, sampler.count);
}



namespace
{
    template <size_t Dims_>
    struct broadcast_sampler
        : public yato::sampler_default
    {
        std::array<size_t, Dims_> masks;

        template <size_t Dim_>
        YATO_CONSTEXPR_FUNC
        bool check_index(std::size_t /*in_idx*/, std::size_t /*size*/) const
        {
            return true;
        }

        template <size_t Dim_>
        YATO_CONSTEXPR_FUNC
        std::size_t transform_index(std::size_t in_idx, std::size_t /*size*/) const
        {
            return in_idx & masks[Dim_];
        }
    };

}

TEST(Yato_Sampler, custom_sampler2)
{
    yato::vector_nd<int, 3> v1(yato::dims(1, 1, 3));
    v1[0][0][0] = 1;
    v1[0][0][1] = 2;
    v1[0][0][2] = 3;

    yato::vector_nd<int, 3> v2(yato::dims(3, 3, 3));

    broadcast_sampler<3> sampler;
    sampler.masks = { 0x0, 0x0, static_cast<size_t>(-1) };

    for (size_t i = 0; i < 3; ++i) {
        for (size_t j = 0; j < 3; ++j) {
            for (size_t k = 0; k < 3; ++k) {
                v2[i][j][k] = yato::loads(sampler, v1, i, j, k);
            }
        }
    }

    for (size_t i = 0; i < 3; ++i) {
        for (size_t j = 0; j < 3; ++j) {
            for (size_t k = 0; k < 3; ++k) {
                EXPECT_EQ(k + 1, v2[i][j][k]);
            }
        }
    }

    static_assert(std::is_same<typename yato::container_traits<decltype(v1)>::container_category, yato::container_continuous_tag>::value);
    static_assert(std::is_same<typename yato::container_traits<decltype(v1[0])>::container_category, yato::container_continuous_tag>::value);
}

