/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2020 Alexey Gruzdev
 */

#include "gtest/gtest.h"

#include <array>
#include <list>
#include <map>
#include <vector>
#include "yato/container_nd.h"
#include "yato/array_nd.h"
#include "yato/vector_nd.h"

namespace
{
    template <typename Cy_>
    auto test_write(Cy_&& dst)
        -> std::enable_if_t<yato::container_traits<yato::remove_cvref_t<Cy_>>::dimensions_number == 2>
    {
        using ops = yato::container_ops<yato::remove_cvref_t<Cy_>>;

        EXPECT_TRUE(ops::continuous(dst));
        EXPECT_TRUE(ops::size(dst, 0) > 1);
        EXPECT_TRUE(ops::size(dst, 1) > 1);
        EXPECT_TRUE(ops::stride(dst, 0) > 1);
        yato::view(dst)[0][0] = 42;
        for(auto it = std::next(ops::begin(dst)); it != ops::end(dst); ++it) {
            (*it)[0] = 10;
        }
        *(ops::data(dst) + 1) = 20;

        auto v = yato::view(dst);
        static_assert(!std::is_const<typename decltype(v)::value_type>::value, "Fail");
        EXPECT_TRUE(v.size(0)   == ops::size(dst, 0));
        EXPECT_TRUE(v.size(1)   == ops::size(dst, 1));
        EXPECT_TRUE(v.stride(0) == ops::stride(dst, 0));
    }

    template <typename Cy_>
    auto test_read(const Cy_& src)
        -> std::enable_if_t<yato::container_traits<yato::remove_cvref_t<Cy_>>::dimensions_number == 2>
    {
        using ops = yato::container_ops<yato::remove_cvref_t<Cy_>>;

        EXPECT_EQ(42, yato::cview(src)[0][0]);
        EXPECT_EQ(42, yato::load(src, 0, 0));
        for(auto it = std::next(ops::cbegin(src)); it != ops::cend(src); ++it) {
            EXPECT_EQ(10, (*it)[0]);
        }
        EXPECT_EQ(20, *(ops::cdata(src) + 1));

        auto v = yato::cview(src);
        static_assert(std::is_const<typename decltype(v)::value_type>::value, "Fail");
        EXPECT_TRUE(v.size(0)   == ops::size(src, 0));
        EXPECT_TRUE(v.size(1)   == ops::size(src, 1));
        EXPECT_TRUE(v.stride(0) == ops::stride(src, 0));
    }


    template <typename Cy_>
    auto test_write(Cy_&& dst)
        -> std::enable_if_t<yato::container_traits<yato::remove_cvref_t<Cy_>>::dimensions_number == 1>
    {
        using ops = yato::container_ops<yato::remove_cvref_t<Cy_>>;

        EXPECT_TRUE(ops::continuous(dst));
        EXPECT_TRUE(ops::size(dst, 0) > 1);
        ops::subscript(dst, 0) = 42;
        for(auto it = std::next(ops::begin(dst), 2); it != ops::end(dst); ++it) {
            (*it) = 10;
        }
        *(ops::data(dst) + 1) = 20;

        auto v = yato::view(dst);
        static_assert(!std::is_const<typename decltype(v)::value_type>::value, "Fail");
        EXPECT_TRUE(v.size(0) == ops::size(dst));
    }

    template <typename Cy_>
    auto test_read(const Cy_& src)
        -> std::enable_if_t<yato::container_traits<yato::remove_cvref_t<Cy_>>::dimensions_number == 1>
    {
        using ops = yato::container_ops<yato::remove_cvref_t<Cy_>>;

        EXPECT_EQ(42, ops::csubscript(src, 0));
        EXPECT_EQ(42, yato::load(src, 0));
        for(auto it = std::next(ops::cbegin(src), 2); it != ops::cend(src); ++it) {
            EXPECT_EQ(10, (*it));
        }
        EXPECT_EQ(20, *(ops::cdata(src) + 1));

        auto v = yato::cview(src);
        static_assert(std::is_const<typename decltype(v)::value_type>::value, "Fail");
        EXPECT_TRUE(v.size(0) == ops::size(src));
    }
}

TEST(Yato_ContainerTraits, common)
{
    using proxy2d_t = yato::remove_cvref_t<decltype(std::declval<yato::vector_3d<float>>()[0])>;
    using subarray2d_t = yato::remove_cvref_t<decltype(std::declval<yato::array_nd<float, 2, 2, 2>>()[0])>;

    static_assert(std::is_same<yato::container_traits<yato::vector_2d<int>>::value_type, int>::value, "invalid container_traits::value_type");
    static_assert(std::is_same<yato::container_traits<yato::array_view_3d<const int>>::value_type, const int>::value, "invalid container_traits::value_type");
    static_assert(std::is_same<yato::container_traits<yato::array_view_1d<float>>::value_type, float>::value, "invalid container_traits::value_type");
    static_assert(std::is_same<yato::container_traits<proxy2d_t>::value_type, float>::value, "invalid container_traits::value_type");
    static_assert(std::is_same<yato::container_traits<subarray2d_t>::value_type, float>::value, "invalid container_traits::value_type");
    static_assert(std::is_same<yato::container_traits<yato::array_nd<int, 2, 2, 2, 2>>::value_type, int>::value, "invalid container_traits::value_type");
    static_assert(std::is_same<yato::container_traits<std::vector<double>>::value_type, double>::value, "invalid container_traits::value_type");
    static_assert(std::is_same<yato::container_traits<std::array<int, 4>>::value_type, int>::value, "invalid container_traits::value_type");
    static_assert(std::is_same<yato::container_traits<int[10]>::value_type, int>::value, "invalid container_traits::value_type");

    static_assert(std::is_same<yato::container_traits<yato::vector_2d<int>>::size_type, std::size_t>::value, "invalid container_traits::size_type");
    static_assert(std::is_same<yato::container_traits<yato::array_view_3d<const int>>::size_type, std::size_t>::value, "invalid container_traits::size_type");
    static_assert(std::is_same<yato::container_traits<yato::array_view_1d<float>>::size_type, std::size_t>::value, "invalid container_traits::size_type");
    static_assert(std::is_same<yato::container_traits<proxy2d_t>::size_type, std::size_t>::value, "invalid container_traits::size_type");
    static_assert(std::is_same<yato::container_traits<subarray2d_t>::size_type, std::size_t>::value, "invalid container_traits::size_type");
    static_assert(std::is_same<yato::container_traits<yato::array_nd<int, 2, 2, 2, 2>>::size_type, std::size_t>::value, "invalid container_traits::value_type");
    static_assert(std::is_same<yato::container_traits<std::vector<double>>::size_type, std::size_t>::value, "invalid container_traits::size_type");
    static_assert(std::is_same<yato::container_traits<std::array<int, 4>>::size_type, std::size_t>::value, "invalid container_traits::size_type");
    static_assert(std::is_same<yato::container_traits<int[10]>::size_type, std::size_t>::value, "invalid container_traits::size_type");

    static_assert(std::is_same<yato::container_traits<yato::vector_2d<int>>::allocator_type, std::allocator<int>>::value, "invalid container_traits::allocator_type");
    static_assert(std::is_same<yato::container_traits<yato::array_view_3d<const int>>::allocator_type, void>::value, "invalid container_traits::allocator_type");
    static_assert(std::is_same<yato::container_traits<yato::array_view_1d<float>>::allocator_type, void>::value, "invalid container_traits::allocator_type");
    static_assert(std::is_same<yato::container_traits<proxy2d_t>::allocator_type, void>::value, "invalid container_traits::allocator_type");
    static_assert(std::is_same<yato::container_traits<subarray2d_t>::allocator_type, void>::value, "invalid container_traits::allocator_type");
    static_assert(std::is_same<yato::container_traits<yato::array_nd<int, 2, 2, 2, 2>>::allocator_type, void>::value, "invalid container_traits::allocator_type");
    static_assert(std::is_same<yato::container_traits<std::vector<double>>::allocator_type, std::allocator<double>>::value, "invalid container_traits::allocator_type");
    static_assert(std::is_same<yato::container_traits<std::array<int, 4>>::allocator_type, void>::value, "invalid container_traits::allocator_type");
    static_assert(std::is_same<yato::container_traits<int[10]>::allocator_type, void>::value, "invalid container_traits::allocator_type");

    static_assert(yato::container_traits<yato::vector_2d<int>>::dimensions_number == 2, "invalid container_traits::dimensions_number");
    static_assert(yato::container_traits<yato::array_view_3d<const int>>::dimensions_number == 3, "invalid container_traits::dimensions_number");
    static_assert(yato::container_traits<yato::array_view_1d<float>>::dimensions_number == 1, "invalid container_traits::dimensions_number");
    static_assert(yato::container_traits<proxy2d_t>::dimensions_number == 2, "invalid container_traits::dimensions_number");
    static_assert(yato::container_traits<subarray2d_t>::dimensions_number == 2, "invalid container_traits::dimensions_number");
    static_assert(yato::container_traits<yato::array_nd<int, 2, 2, 2, 2>>::dimensions_number == 4, "invalid container_traits::dimensions_number");
    static_assert(yato::container_traits<std::vector<double>>::dimensions_number == 1, "invalid container_traits::dimensions_number");
    static_assert(yato::container_traits<std::array<int, 4>>::dimensions_number == 1, "invalid container_traits::dimensions_number");
    static_assert(yato::container_traits<int[10]>::dimensions_number == 1, "invalid container_traits::dimensions_number");

    static_assert(std::is_same<yato::container_traits<yato::vector_2d<int>>::dimensions_type, yato::dimensionality<2>>::value, "invalid container_traits::dimensions_type");
    static_assert(std::is_same<yato::container_traits<yato::array_view_3d<const int>>::dimensions_type, yato::dimensionality<3>>::value, "invalid container_traits::dimensions_type");
    static_assert(std::is_same<yato::container_traits<yato::array_view_1d<float>>::dimensions_type, yato::dimensionality<1>>::value, "invalid container_traits::dimensions_type");
    static_assert(std::is_same<yato::container_traits<proxy2d_t>::dimensions_type, yato::dimensionality<2>>::value, "invalid container_traits::dimensions_type");
    static_assert(std::is_same<yato::container_traits<subarray2d_t>::dimensions_type, yato::dimensionality<2>>::value, "invalid container_traits::dimensions_type");
    static_assert(std::is_same<yato::container_traits<yato::array_nd<int, 2, 2, 2, 2>>::dimensions_type, yato::dimensionality<4>>::value, "invalid container_traits::dimensions_type");
    static_assert(std::is_same<yato::container_traits<std::vector<double>>::dimensions_type, yato::dimensionality<1>>::value, "invalid container_traits::dimensions_type");
    static_assert(std::is_same<yato::container_traits<std::array<int, 4>>::dimensions_type, yato::dimensionality<1>>::value, "invalid container_traits::dimensions_type");
    static_assert(std::is_same<yato::container_traits<int[10]>::dimensions_type, yato::dimensionality<1>>::value, "invalid container_traits::dimensions_type");

    static_assert(std::is_same<yato::container_traits<yato::vector_2d<int>>::strides_type, yato::strides_array<1>>::value, "invalid container_traits::strides_type");
    static_assert(std::is_same<yato::container_traits<yato::array_view_3d<const int>>::strides_type, yato::strides_array<2>>::value, "invalid container_traits::strides_type");
    static_assert(std::is_same<yato::container_traits<yato::array_view_1d<float>>::strides_type, yato::strides_array<0>>::value, "invalid container_traits::strides_type");
    static_assert(std::is_same<yato::container_traits<proxy2d_t>::strides_type, yato::strides_array<1>>::value, "invalid container_traits::strides_type");
    static_assert(std::is_same<yato::container_traits<subarray2d_t>::strides_type, yato::strides_array<1>>::value, "invalid container_traits::strides_type");
    static_assert(std::is_same<yato::container_traits<yato::array_nd<int, 2, 2, 2, 2>>::strides_type, yato::strides_array<3>>::value, "invalid container_traits::strides_type");
    static_assert(std::is_same<yato::container_traits<std::vector<double>>::strides_type, yato::strides_array<0>>::value, "invalid container_traits::strides_type");
    static_assert(std::is_same<yato::container_traits<std::array<int, 4>>::strides_type, yato::strides_array<0>>::value, "invalid container_traits::strides_type");
    static_assert(std::is_same<yato::container_traits<int[10]>::strides_type, yato::strides_array<0>>::value, "invalid container_traits::strides_type");

    static_assert(std::is_same<yato::container_traits<yato::vector<int>>::iterator, int*>::value, "invalid container_traits::iterator");
    static_assert(std::is_same<yato::container_traits<yato::vector<int>>::const_iterator, const int*>::value, "invalid container_traits::const_iterator");
    static_assert(std::is_same<yato::container_traits<yato::vector_2d<int>>::plain_iterator, int*>::value, "invalid container_traits::plain_iterator");
    static_assert(std::is_same<yato::container_traits<yato::vector_2d<int>>::const_plain_iterator, const int*>::value, "invalid container_traits::const_plain_iterator");
    static_assert(std::is_same<yato::container_traits<yato::array_view_3d<const int>>::plain_iterator, const int*>::value, "invalid container_traits::plain_iterator");
    static_assert(std::is_same<yato::container_traits<yato::array_view_3d<const int>>::const_plain_iterator, const int*>::value, "invalid container_traits::const_plain_iterator");
    static_assert(std::is_same<yato::container_traits<yato::array_view_1d<float>>::iterator, float*>::value, "invalid container_traits::iterator");
    static_assert(std::is_same<yato::container_traits<yato::array_view_1d<float>>::const_iterator, const float*>::value, "invalid container_traits::const_iterator");
    static_assert(std::is_same<yato::container_traits<yato::array_view_1d<float>>::plain_iterator, float*>::value, "invalid container_traits::plain_iterator");
    static_assert(std::is_same<yato::container_traits<yato::array_view_1d<float>>::const_plain_iterator, const float*>::value, "invalid container_traits::const_plain_iterator");
    static_assert(std::is_same<yato::container_traits<proxy2d_t>::plain_iterator, float*>::value, "invalid container_traits::plain_iterator");
    static_assert(std::is_same<yato::container_traits<proxy2d_t>::const_plain_iterator, const float*>::value, "invalid container_traits::const_plain_iterator");
    static_assert(std::is_same<yato::container_traits<subarray2d_t>::plain_iterator, float*>::value, "invalid container_traits::plain_iterator");
    static_assert(std::is_same<yato::container_traits<subarray2d_t>::const_plain_iterator, const float*>::value, "invalid container_traits::const_plain_iterator");
    static_assert(std::is_same<yato::container_traits<yato::array_nd<int, 2, 2, 2, 2>>::plain_iterator, int*>::value, "invalid container_traits::plain_iterator");
    static_assert(std::is_same<yato::container_traits<yato::array_nd<int, 2, 2, 2, 2>>::const_plain_iterator, const int*>::value, "invalid container_traits::const_plain_iterator");
    static_assert(std::is_same<yato::container_traits<std::vector<double>>::iterator, std::vector<double>::iterator>::value, "invalid container_traits::iterator");
    static_assert(std::is_same<yato::container_traits<std::vector<double>>::const_iterator, std::vector<double>::const_iterator>::value, "invalid container_traits::const_iterator");
    static_assert(std::is_same<yato::container_traits<std::vector<double>>::plain_iterator, std::vector<double>::iterator>::value, "invalid container_traits::plain_iterator");
    static_assert(std::is_same<yato::container_traits<std::vector<double>>::const_plain_iterator, std::vector<double>::const_iterator>::value, "invalid container_traits::const_plain_iterator");
    static_assert(std::is_same<yato::container_traits<std::array<int, 4>>::iterator, std::array<int, 4>::iterator>::value, "invalid container_traits::iterator");
    static_assert(std::is_same<yato::container_traits<std::array<int, 4>>::const_iterator, std::array<int, 4>::const_iterator>::value, "invalid container_traits::const_iterator");
    static_assert(std::is_same<yato::container_traits<std::array<int, 4>>::plain_iterator, std::array<int, 4>::iterator>::value, "invalid container_traits::plain_iterator");
    static_assert(std::is_same<yato::container_traits<std::array<int, 4>>::const_plain_iterator, std::array<int, 4>::const_iterator>::value, "invalid container_traits::const_plain_iterator");
    static_assert(std::is_same<yato::container_traits<int[10]>::iterator, int*>::value, "invalid container_traits::iterator");
    static_assert(std::is_same<yato::container_traits<int[10]>::const_iterator, const int*>::value, "invalid container_traits::const_iterator");
    static_assert(std::is_same<yato::container_traits<int[10]>::plain_iterator, int*>::value, "invalid container_traits::plain_iterator");
    static_assert(std::is_same<yato::container_traits<int[10]>::const_plain_iterator, const int*>::value, "invalid container_traits::const_plain_iterator");

    static_assert(yato::container_traits<yato::vector_2d<int>>::has_method_continuous == true, "invalid container_traits::has_method_continuous");
    static_assert(yato::container_traits<yato::array_view_3d<const int>>::has_method_continuous == true, "invalid container_traits::has_method_continuous");
    static_assert(yato::container_traits<yato::array_view_1d<float>>::has_method_continuous == true, "invalid container_traits::has_method_continuous");
    static_assert(yato::container_traits<proxy2d_t>::has_method_continuous == true, "invalid container_traits::has_method_continuous");
    static_assert(yato::container_traits<subarray2d_t>::has_method_continuous == true, "invalid container_traits::has_method_continuous");
    static_assert(yato::container_traits<yato::array_nd<int, 2, 2, 2, 2>>::has_method_continuous == true, "invalid container_traits::has_method_continuous");
    static_assert(yato::container_traits<std::vector<double>>::has_method_continuous == false, "invalid container_traits::has_method_continuous");
    static_assert(yato::container_traits<std::array<int, 4>>::has_method_continuous == false, "invalid container_traits::has_method_continuous");

    static_assert(yato::container_traits<yato::vector_2d<int>>::has_operator_subscript == true, "invalid container_traits::has_operator_subscript");
    static_assert(yato::container_traits<yato::vector_2d<std::unique_ptr<int>>>::has_operator_subscript == true, "invalid container_traits::has_operator_subscript");
    static_assert(yato::container_traits<yato::array_view_3d<const int>>::has_operator_subscript == true, "invalid container_traits::has_operator_subscript");
    static_assert(yato::container_traits<yato::array_view_1d<float>>::has_operator_subscript == true, "invalid container_traits::has_operator_subscript");
    static_assert(yato::container_traits<yato::array_view_1d<std::unique_ptr<int>>>::has_operator_subscript == true, "invalid container_traits::has_operator_subscript");
    static_assert(yato::container_traits<proxy2d_t>::has_operator_subscript == true, "invalid container_traits::has_operator_subscript");
    static_assert(yato::container_traits<subarray2d_t>::has_operator_subscript == true, "invalid container_traits::has_operator_subscript");
    static_assert(yato::container_traits<yato::array_nd<int, 2, 2, 2, 2>>::has_operator_subscript == true, "invalid container_traits::has_operator_subscript");
    static_assert(yato::container_traits<std::vector<double>>::has_operator_subscript == true, "invalid container_traits::has_operator_subscript");
    static_assert(yato::container_traits<std::array<int, 4>>::has_operator_subscript == true, "invalid container_traits::has_operator_subscript");

    static_assert(yato::container_traits<yato::vector_2d<int>>::has_method_data == true, "invalid container_traits::has_method_data");
    static_assert(yato::container_traits<yato::vector_2d<std::unique_ptr<int>>>::has_method_data == true, "invalid container_traits::has_method_data");
    static_assert(yato::container_traits<yato::array_view_3d<const int>>::has_method_data == true, "invalid container_traits::has_method_data");
    static_assert(yato::container_traits<yato::array_view_1d<float>>::has_method_data == true, "invalid container_traits::has_method_data");
    static_assert(yato::container_traits<yato::array_view_1d<std::unique_ptr<int>>>::has_method_data == true, "invalid container_traits::has_method_data");
    static_assert(yato::container_traits<proxy2d_t>::has_method_data == true, "invalid container_traits::has_method_data");
    static_assert(yato::container_traits<subarray2d_t>::has_method_data == true, "invalid container_traits::has_method_data");
    static_assert(yato::container_traits<yato::array_nd<int, 2, 2, 2, 2>>::has_method_data == true, "invalid container_traits::has_method_data");
    static_assert(yato::container_traits<std::vector<double>>::has_method_data == true, "invalid container_traits::has_method_data");
    static_assert(yato::container_traits<std::array<int, 4>>::has_method_data == true, "invalid container_traits::has_method_data");

    static_assert(yato::is_container<yato::vector_2d<int>>::value, "invalid is_container");
    static_assert(yato::is_container<yato::array_view_3d<const int>>::value, "invalid is_container");
    static_assert(yato::is_container<yato::array_view_1d<float>>::value, "invalid is_container");
    static_assert(yato::is_container<proxy2d_t>::value, "invalid is_container");
    static_assert(yato::is_container<yato::array_nd<int, 2, 2, 2, 2>>::value, "invalid is_container");
    static_assert(yato::is_container<std::vector<double>>::value, "invalid is_container");
    static_assert(yato::is_container<std::array<int, 4>>::value, "invalid is_container");
    static_assert(yato::is_container<int[10]>::value, "invalid is_container");
}

namespace
{

    template <typename Cy_>
    void test_read_1d(const Cy_& c, bool testing_continuous)
    {
        using ops = yato::container_ops<Cy_>;

        ASSERT_EQ(1, ops::dimensions_number);

        EXPECT_EQ(static_cast<size_t>(4), ops::size(c));
        EXPECT_EQ(static_cast<size_t>(4), ops::size(c, 0));
        EXPECT_EQ(static_cast<size_t>(4), ops::total_size(c));
        EXPECT_EQ(testing_continuous, ops::continuous(c));

        const auto dims = ops::dimensions(c);
        EXPECT_EQ(1, dims.size());
        EXPECT_EQ(4, dims[0]);

        EXPECT_EQ(1, ops::csubscript(c, 0));
        EXPECT_EQ(2, ops::csubscript(c, 1));
        EXPECT_EQ(3, ops::csubscript(c, 2));
        EXPECT_EQ(4, ops::csubscript(c, 3));

        std::array<int, 4> tmp1 = {};
        std::copy(ops::cbegin(c), ops::cend(c), std::begin(tmp1));
        EXPECT_EQ(1, tmp1[0]);
        EXPECT_EQ(2, tmp1[1]);
        EXPECT_EQ(3, tmp1[2]);
        EXPECT_EQ(4, tmp1[3]);
    }

    template <typename Cy_>
    void test_read_1d_memcpy(const Cy_& c)
    {
        using ops = yato::container_ops<Cy_>;

        EXPECT_EQ(true, ops::continuous(c));

        using value_type = std::remove_const_t<typename ops::value_type>;
        std::array<value_type, 4> tmp1 = {};
        std::memcpy(tmp1.data(), ops::cdata(c), ops::total_size(c) * sizeof(value_type));
        EXPECT_TRUE(std::equal(std::cbegin(tmp1), std::cend(tmp1), ops::cbegin(c)));
    }

    template <typename Cy_>
    void test_read_3d(const Cy_& c)
    {
        using ops = yato::container_ops<Cy_>;

        ASSERT_EQ(3, ops::dimensions_number);

        EXPECT_EQ(static_cast<size_t>(2), ops::size(c, 0));
        EXPECT_EQ(static_cast<size_t>(3), ops::size(c, 1));
        EXPECT_EQ(static_cast<size_t>(4), ops::size(c, 2));
        EXPECT_EQ(static_cast<size_t>(24), ops::total_size(c));
        EXPECT_EQ(true, ops::continuous(c));

        const auto dims = ops::dimensions(c);
        EXPECT_EQ(3, dims.size());
        EXPECT_EQ(2, dims[0]);
        EXPECT_EQ(3, dims[1]);
        EXPECT_EQ(4, dims[2]);

        constexpr size_t elem_size = sizeof(typename ops::value_type);
        EXPECT_EQ(static_cast<size_t>(12 * elem_size), ops::stride(c, 0));
        EXPECT_EQ(static_cast<size_t>( 4 * elem_size), ops::stride(c, 1));

        const auto strides = ops::strides(c);
        EXPECT_EQ(static_cast<size_t>(12 * elem_size), strides[0]);
        EXPECT_EQ(static_cast<size_t>( 4 * elem_size), strides[1]);

        const auto view = yato::cview(c);
        EXPECT_EQ(1, view[0][0][0]);
        EXPECT_EQ(2, view[0][0][1]);
        EXPECT_EQ(3, view[0][0][2]);
        EXPECT_EQ(4, view[0][0][3]);

        EXPECT_EQ(5, view[0][1][0]);
        EXPECT_EQ(6, view[0][1][1]);
        EXPECT_EQ(7, view[0][1][2]);
        EXPECT_EQ(8, view[0][1][3]);

        EXPECT_EQ(9,  view[0][2][0]);
        EXPECT_EQ(10, view[0][2][1]);
        EXPECT_EQ(11, view[0][2][2]);
        EXPECT_EQ(12, view[0][2][3]);

        EXPECT_EQ(13, view[1][0][0]);
        EXPECT_EQ(14, view[1][0][1]);
        EXPECT_EQ(15, view[1][0][2]);
        EXPECT_EQ(16, view[1][0][3]);

        EXPECT_EQ(17, view[1][1][0]);
        EXPECT_EQ(18, view[1][1][1]);
        EXPECT_EQ(19, view[1][1][2]);
        EXPECT_EQ(20, view[1][1][3]);

        EXPECT_EQ(21, view[1][2][0]);
        EXPECT_EQ(22, view[1][2][1]);
        EXPECT_EQ(23, view[1][2][2]);
        EXPECT_EQ(24, view[1][2][3]);
    }

} // namespace

TEST(Yato_ContainerTraits, read_1d)
{
    std::array<int, 4> stdArr = {1, 2, 3, 4};
    std::vector<int> stdVec = {1, 2, 3, 4};
    //int stdArr4[4] = {1, 2, 3, 4};
    yato::vector_1d<int> yatoVec = {1, 2, 3, 4};
    yato::array_nd<int, 4> yatoArr = {1, 2, 3, 4};
    yato::array_view_1d<int> yatoView = yato::view(yatoVec);
    yato::array_view_1d<const int> yatoCView = yato::cview(yatoVec);

    test_read_1d(stdArr, true);
    test_read_1d(stdVec, true);
    //test_read_1d(stdArr4, false);
    test_read_1d(yatoVec, true);
    test_read_1d(yatoArr, true);
    test_read_1d(yatoView, true);
    test_read_1d(yatoCView, true);

    test_read_1d_memcpy(stdArr);
    test_read_1d_memcpy(stdVec);
    test_read_1d_memcpy(yatoVec);
    test_read_1d_memcpy(yatoArr);
    test_read_1d_memcpy(yatoView);
    test_read_1d_memcpy(yatoCView);

    yato::vector_2d<int> yatoVec2d = { {1, 2, 3, 4}, {5, 6, 7, 8} };
    yato::array_nd<int, 2, 4> yatoArr2d = {{ {1, 2, 3, 4}, {5, 6, 7, 8} }};
    test_read_1d(yatoVec2d[0], true);
    test_read_1d(yatoArr2d[0], true);
    test_read_1d_memcpy(yatoVec2d[1]);
    test_read_1d_memcpy(yatoArr2d[1]);
}

TEST(Yato_ContainerTraits, read_3d)
{
    yato::array_nd<int, 2, 3, 4> yatoArr = {{
        {{1, 2, 3, 4},
         {5, 6, 7, 8},
         {9, 10, 11, 12}},

        {{13, 14, 15, 16},
         {17, 18, 19, 20},
         {21, 22, 23, 24}}
    }};

     yato::vector_3d<int> yatoVec = {
        {{1, 2, 3, 4},
         {5, 6, 7, 8},
         {9, 10, 11, 12}},

        {{13, 14, 15, 16},
         {17, 18, 19, 20},
         {21, 22, 23, 24}}
    };

    yato::array_view_3d<int> yatoView = yato::view(yatoVec);
    yato::array_view_3d<const int> yatoCView = yato::cview(yatoVec);

    test_read_3d(yatoVec);
    test_read_3d(yatoArr);
    test_read_3d(yatoView);
    test_read_3d(yatoCView);
}



TEST(Yato_ContainerND, access)
{
    yato::vector_nd<int, 2> v2(yato::dims(4, 4), -1);
    test_write(v2);
    test_write(v2.proxy());
    test_write(v2.view());
    //test_write(v2.cview());  // can't compile -> good

    const yato::array_view_2d<int> t(v2.view());
    test_write(t);

    yato::vector_nd<int, 3> v3(yato::dims(4, 4, 4), -1);
    test_write(v3[1]);

    test_read(v2);
    test_read(v2.view());
    test_read(v2.cview());
    test_read(v3[1]);

    EXPECT_EQ(42, v2[0][0]);
    EXPECT_EQ(10, v2[1][0]);
    EXPECT_EQ(10, v2[2][0]);
    EXPECT_EQ(10, v2[3][0]);
}

TEST(Yato_ContainerND, access_array)
{
    yato::array_nd<int, 4, 4> arr2;
    arr2.fill(-1);

    auto v = yato::view(arr2);
    test_write(arr2);
    test_write(v);

    test_read(arr2);
    test_read(v);

    EXPECT_EQ(42, arr2[0][0]);
    EXPECT_EQ(10, arr2[1][0]);
    EXPECT_EQ(10, arr2[2][0]);
    EXPECT_EQ(10, arr2[3][0]);
}

TEST(Yato_ContainerND, access_1)
{
    yato::vector_nd<int, 1> v1(yato::dims(8), -1);
    test_write(v1);
    test_write(v1.proxy());
    test_write(v1.view());
    //test_write(v1.cview());  // can't compile -> good

    const yato::array_view_1d<int> t(v1.view());
    test_write(t);

    yato::vector_nd<int, 2> v2(yato::dims(8, 8), -1);
    test_write(v2[1]);

    test_read(v1);
    test_read(v1.view());
    test_read(v1.cview());
    test_read(v2[1]);

    EXPECT_EQ(42, v1[0]);
    EXPECT_EQ(20, v1[1]);
    EXPECT_EQ(10, v1[2]);
    EXPECT_EQ(10, v1[3]);
    EXPECT_EQ(10, v1[4]);
    EXPECT_EQ(10, v1[5]);
    EXPECT_EQ(10, v1[6]);
    EXPECT_EQ(10, v1[7]);
}
