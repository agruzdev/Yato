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
    template <typename Ty_, typename Impl_>
    void test_write(const yato::container_nd<Ty_, 2, Impl_> & dst)
    {
        EXPECT_TRUE(dst.continuous());
        EXPECT_TRUE(dst.size(0) > 1);
        EXPECT_TRUE(dst.size(1) > 1);
        EXPECT_TRUE(dst.stride(0) > 1);
        dst[0][0] = 42;
        for(auto it = std::next(dst.begin()); it != dst.end(); ++it) {
            (*it)[0] = 10;
        }
        *(dst.data() + 1) = 20;

        auto v = yato::view(dst);
        static_assert(!std::is_const<typename decltype(v)::value_type>::value, "Fail");
        EXPECT_TRUE(v.size(0)   == dst.size(0));
        EXPECT_TRUE(v.size(1)   == dst.size(1));
        EXPECT_TRUE(v.stride(0) == dst.stride(0));
    }

    template <typename Ty_, typename Impl_>
    void test_read(const yato::const_container_nd<Ty_, 2, Impl_> & src)
    {
        EXPECT_EQ(42, src[0][0]);
        for(auto it = std::next(src.cbegin()); it != src.cend(); ++it) {
            EXPECT_EQ(10, (*it)[0]);
        }
        EXPECT_EQ(20, *(src.cdata() + 1));

        auto v = yato::cview(src);
        static_assert(std::is_const<typename decltype(v)::value_type>::value, "Fail");
        EXPECT_TRUE(v.size(0)   == src.size(0));
        EXPECT_TRUE(v.size(1)   == src.size(1));
        EXPECT_TRUE(v.stride(0) == src.stride(0));
    }


    template <typename Ty_, typename Impl_>
    void test_write(const yato::container_nd<Ty_, 1, Impl_> & dst)
    {
        EXPECT_TRUE(dst.continuous());
        EXPECT_TRUE(dst.size(0) > 1);
        dst[0] = 42;
        for(auto it = std::next(dst.begin(), 2); it != dst.end(); ++it) {
            (*it) = 10;
        }
        *(dst.data() + 1) = 20;

        auto v = yato::view(dst);
        static_assert(!std::is_const<typename decltype(v)::value_type>::value, "Fail");
        EXPECT_TRUE(v.size(0) == dst.size(0));
    }

    template <typename Ty_, typename Impl_>
    void test_read(const yato::const_container_nd<Ty_, 1, Impl_> & src)
    {
        EXPECT_EQ(42, src[0]);
        for(auto it = std::next(src.cbegin(), 2); it != src.cend(); ++it) {
            EXPECT_EQ(10, (*it));
        }
        EXPECT_EQ(20, *(src.cdata() + 1));

        auto v = yato::cview(src);
        static_assert(std::is_const<typename decltype(v)::value_type>::value, "Fail");
        EXPECT_TRUE(v.size(0) == src.size(0));
    }
}

TEST(Yato_ContainerTraits, common)
{
    using proxy2d_t = yato::remove_cvref_t<decltype(std::declval<yato::vector_3d<float>>()[0])>;
    using subarray2d_t = yato::remove_cvref_t<decltype(std::declval<yato::array_nd<float, 2, 2, 2>>()[0])>;

    static_assert(yato::container_traits<yato::vector_2d<int>>::container_category == yato::container_tag::continuous, "invalid container_traits::container_category");
    static_assert(yato::container_traits<yato::array_view_3d<const int>>::container_category == yato::container_tag::general, "invalid container_traits::container_category");
    static_assert(yato::container_traits<yato::array_view_1d<float>>::container_category == yato::container_tag::continuous, "invalid container_traits::container_category");
    static_assert(yato::container_traits<proxy2d_t>::container_category == yato::container_tag::general, "invalid container_traits::container_category");
    static_assert(yato::container_traits<subarray2d_t>::container_category == yato::container_tag::continuous, "invalid container_traits::container_category");
    static_assert(yato::container_traits<yato::array_nd<int, 2, 2, 2, 2>>::container_category == yato::container_tag::continuous, "invalid container_traits::container_category");
    static_assert(yato::container_traits<std::vector<double>>::container_category == yato::container_tag::continuous, "invalid container_traits::container_category");
    static_assert(yato::container_traits<std::array<int, 4>>::container_category == yato::container_tag::continuous, "invalid container_traits::container_category");
    static_assert(yato::container_traits<std::list<short>>::container_category == yato::container_tag::general, "invalid container_traits::container_category");
    static_assert(yato::container_traits<std::map<std::string, int>>::container_category == yato::container_tag::general, "invalid container_traits::container_category");

    static_assert(std::is_same<yato::container_traits<yato::vector_2d<int>>::value_type, int>::value, "invalid container_traits::value_type");
    static_assert(std::is_same<yato::container_traits<yato::array_view_3d<const int>>::value_type, const int>::value, "invalid container_traits::value_type");
    static_assert(std::is_same<yato::container_traits<yato::array_view_1d<float>>::value_type, float>::value, "invalid container_traits::value_type");
    static_assert(std::is_same<yato::container_traits<proxy2d_t>::value_type, float>::value, "invalid container_traits::value_type");
    static_assert(std::is_same<yato::container_traits<subarray2d_t>::value_type, float>::value, "invalid container_traits::value_type");
    static_assert(std::is_same<yato::container_traits<yato::array_nd<int, 2, 2, 2, 2>>::value_type, int>::value, "invalid container_traits::value_type");
    static_assert(std::is_same<yato::container_traits<std::vector<double>>::value_type, double>::value, "invalid container_traits::value_type");
    static_assert(std::is_same<yato::container_traits<std::array<int, 4>>::value_type, int>::value, "invalid container_traits::value_type");
    static_assert(std::is_same<yato::container_traits<std::list<short>>::value_type, short>::value, "invalid container_traits::value_type");
    static_assert(std::is_same<yato::container_traits<std::map<std::string, int>>::value_type, std::pair<const std::string, int>>::value, "invalid container_traits::value_type");

    static_assert(std::is_same<yato::container_traits<yato::vector_2d<int>>::size_type, std::size_t>::value, "invalid container_traits::size_type");
    static_assert(std::is_same<yato::container_traits<yato::array_view_3d<const int>>::size_type, std::size_t>::value, "invalid container_traits::size_type");
    static_assert(std::is_same<yato::container_traits<yato::array_view_1d<float>>::size_type, std::size_t>::value, "invalid container_traits::size_type");
    static_assert(std::is_same<yato::container_traits<proxy2d_t>::size_type, std::size_t>::value, "invalid container_traits::size_type");
    static_assert(std::is_same<yato::container_traits<subarray2d_t>::size_type, std::size_t>::value, "invalid container_traits::size_type");
    static_assert(std::is_same<yato::container_traits<yato::array_nd<int, 2, 2, 2, 2>>::size_type, std::size_t>::value, "invalid container_traits::value_type");
    static_assert(std::is_same<yato::container_traits<std::vector<double>>::size_type, std::size_t>::value, "invalid container_traits::size_type");
    static_assert(std::is_same<yato::container_traits<std::array<int, 4>>::size_type, std::size_t>::value, "invalid container_traits::size_type");
    static_assert(std::is_same<yato::container_traits<std::list<short>>::size_type, std::size_t>::value, "invalid container_traits::size_type");
    static_assert(std::is_same<yato::container_traits<std::map<std::string, int>>::size_type, std::size_t>::value, "invalid container_traits::size_type");

    static_assert(std::is_same<yato::container_traits<yato::vector_2d<int>>::allocator_type, std::allocator<int>>::value, "invalid container_traits::allocator_type");
    static_assert(std::is_same<yato::container_traits<yato::array_view_3d<const int>>::allocator_type, void>::value, "invalid container_traits::allocator_type");
    static_assert(std::is_same<yato::container_traits<yato::array_view_1d<float>>::allocator_type, void>::value, "invalid container_traits::allocator_type");
    static_assert(std::is_same<yato::container_traits<proxy2d_t>::allocator_type, void>::value, "invalid container_traits::allocator_type");
    static_assert(std::is_same<yato::container_traits<subarray2d_t>::allocator_type, void>::value, "invalid container_traits::allocator_type");
    static_assert(std::is_same<yato::container_traits<yato::array_nd<int, 2, 2, 2, 2>>::allocator_type, void>::value, "invalid container_traits::allocator_type");
    static_assert(std::is_same<yato::container_traits<std::vector<double>>::allocator_type, std::allocator<double>>::value, "invalid container_traits::allocator_type");
    static_assert(std::is_same<yato::container_traits<std::array<int, 4>>::allocator_type, void>::value, "invalid container_traits::allocator_type");
    static_assert(std::is_same<yato::container_traits<std::list<short>>::allocator_type, std::allocator<short>>::value, "invalid container_traits::allocator_type");
    static_assert(std::is_same<yato::container_traits<std::map<std::string, int>>::allocator_type, std::allocator<std::pair<const std::string, int>>>::value, "invalid container_traits::allocator_type");

    static_assert(yato::container_traits<yato::vector_2d<int>>::dimensions_number == 2, "invalid container_traits::dimensions_number");
    static_assert(yato::container_traits<yato::array_view_3d<const int>>::dimensions_number == 3, "invalid container_traits::dimensions_number");
    static_assert(yato::container_traits<yato::array_view_1d<float>>::dimensions_number == 1, "invalid container_traits::dimensions_number");
    static_assert(yato::container_traits<proxy2d_t>::dimensions_number == 2, "invalid container_traits::dimensions_number");
    static_assert(yato::container_traits<subarray2d_t>::dimensions_number == 2, "invalid container_traits::dimensions_number");
    static_assert(yato::container_traits<yato::array_nd<int, 2, 2, 2, 2>>::dimensions_number == 4, "invalid container_traits::dimensions_number");
    static_assert(yato::container_traits<std::vector<double>>::dimensions_number == 1, "invalid container_traits::dimensions_number");
    static_assert(yato::container_traits<std::array<int, 4>>::dimensions_number == 1, "invalid container_traits::dimensions_number");
    static_assert(yato::container_traits<std::list<short>>::dimensions_number == 1, "invalid container_traits::dimensions_number");
    static_assert(yato::container_traits<std::map<std::string, int>>::dimensions_number == 1, "invalid container_traits::dimensions_number");

    static_assert(std::is_same<yato::container_traits<yato::vector_2d<int>>::dimensions_type, yato::dimensionality<2>>::value, "invalid container_traits::dimensions_type");
    static_assert(std::is_same<yato::container_traits<yato::array_view_3d<const int>>::dimensions_type, yato::dimensionality<3>>::value, "invalid container_traits::dimensions_type");
    static_assert(std::is_same<yato::container_traits<yato::array_view_1d<float>>::dimensions_type, yato::dimensionality<1>>::value, "invalid container_traits::dimensions_type");
    static_assert(std::is_same<yato::container_traits<proxy2d_t>::dimensions_type, yato::dimensionality<2>>::value, "invalid container_traits::dimensions_type");
    static_assert(std::is_same<yato::container_traits<subarray2d_t>::dimensions_type, yato::dimensionality<2>>::value, "invalid container_traits::dimensions_type");
    static_assert(std::is_same<yato::container_traits<yato::array_nd<int, 2, 2, 2, 2>>::dimensions_type, yato::dimensionality<4>>::value, "invalid container_traits::dimensions_type");
    static_assert(std::is_same<yato::container_traits<std::vector<double>>::dimensions_type, yato::dimensionality<1>>::value, "invalid container_traits::dimensions_type");
    static_assert(std::is_same<yato::container_traits<std::array<int, 4>>::dimensions_type, yato::dimensionality<1>>::value, "invalid container_traits::dimensions_type");
    static_assert(std::is_same<yato::container_traits<std::list<short>>::dimensions_type, yato::dimensionality<1>>::value, "invalid container_traits::dimensions_type");
    static_assert(std::is_same<yato::container_traits<std::map<std::string, int>>::dimensions_type, yato::dimensionality<1>>::value, "invalid container_traits::dimensions_type");

    static_assert(std::is_same<yato::container_traits<yato::vector_2d<int>>::strides_type, yato::strides_array<1>>::value, "invalid container_traits::strides_type");
    static_assert(std::is_same<yato::container_traits<yato::array_view_3d<const int>>::strides_type, yato::strides_array<2>>::value, "invalid container_traits::strides_type");
    static_assert(std::is_same<yato::container_traits<yato::array_view_1d<float>>::strides_type, yato::strides_array<0>>::value, "invalid container_traits::strides_type");
    static_assert(std::is_same<yato::container_traits<proxy2d_t>::strides_type, yato::strides_array<1>>::value, "invalid container_traits::strides_type");
    static_assert(std::is_same<yato::container_traits<subarray2d_t>::strides_type, yato::strides_array<1>>::value, "invalid container_traits::strides_type");
    static_assert(std::is_same<yato::container_traits<yato::array_nd<int, 2, 2, 2, 2>>::strides_type, yato::strides_array<3>>::value, "invalid container_traits::strides_type");
    static_assert(std::is_same<yato::container_traits<std::vector<double>>::strides_type, yato::strides_array<0>>::value, "invalid container_traits::strides_type");
    static_assert(std::is_same<yato::container_traits<std::array<int, 4>>::strides_type, yato::strides_array<0>>::value, "invalid container_traits::strides_type");
    static_assert(std::is_same<yato::container_traits<std::list<short>>::strides_type, yato::strides_array<0>>::value, "invalid container_traits::strides_type");
    static_assert(std::is_same<yato::container_traits<std::map<std::string, int>>::strides_type, yato::strides_array<0>>::value, "invalid container_traits::strides_type");

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
    static_assert(std::is_same<yato::container_traits<std::list<short>>::iterator, std::list<short>::iterator>::value, "invalid container_traits::iterator");
    static_assert(std::is_same<yato::container_traits<std::list<short>>::const_iterator, std::list<short>::const_iterator>::value, "invalid container_traits::const_iterator");
    static_assert(std::is_same<yato::container_traits<std::list<short>>::plain_iterator, std::list<short>::iterator>::value, "invalid container_traits::plain_iterator");
    static_assert(std::is_same<yato::container_traits<std::list<short>>::const_plain_iterator, std::list<short>::const_iterator>::value, "invalid container_traits::const_plain_iterator");
    static_assert(std::is_same<yato::container_traits<std::map<std::string, int>>::iterator, std::map<std::string, int>::iterator>::value, "invalid container_traits::iterator");
    static_assert(std::is_same<yato::container_traits<std::map<std::string, int>>::const_iterator, std::map<std::string, int>::const_iterator>::value, "invalid container_traits::const_iterator");
    static_assert(std::is_same<yato::container_traits<std::map<std::string, int>>::plain_iterator, std::map<std::string, int>::iterator>::value, "invalid container_traits::plain_iterator");
    static_assert(std::is_same<yato::container_traits<std::map<std::string, int>>::const_plain_iterator, std::map<std::string, int>::const_iterator>::value, "invalid container_traits::const_plain_iterator");

    static_assert(yato::container_traits<yato::vector_2d<int>>::has_continuous == true, "invalid container_traits::container_category");
    static_assert(yato::container_traits<yato::array_view_3d<const int>>::has_continuous == true, "invalid container_traits::container_category");
    static_assert(yato::container_traits<yato::array_view_1d<float>>::has_continuous == true, "invalid container_traits::container_category");
    static_assert(yato::container_traits<proxy2d_t>::has_continuous == true, "invalid container_traits::container_category");
    static_assert(yato::container_traits<subarray2d_t>::has_continuous == false, "invalid container_traits::container_category");
    static_assert(yato::container_traits<yato::array_nd<int, 2, 2, 2, 2>>::has_continuous == false, "invalid container_traits::container_category");
    static_assert(yato::container_traits<std::vector<double>>::has_continuous == false, "invalid container_traits::container_category");
    static_assert(yato::container_traits<std::array<int, 4>>::has_continuous == false, "invalid container_traits::container_category");
    static_assert(yato::container_traits<std::list<short>>::has_continuous == false, "invalid container_traits::container_category");
    static_assert(yato::container_traits<std::map<std::string, int>>::has_continuous == false, "invalid container_traits::container_category");

    static_assert(yato::is_container<yato::vector_2d<int>>::value, "invalid is_container");
    static_assert(yato::is_container<yato::array_view_3d<const int>>::value, "invalid is_container");
    static_assert(yato::is_container<yato::array_view_1d<float>>::value, "invalid is_container");
    static_assert(yato::is_container<proxy2d_t>::value, "invalid is_container");
    static_assert(yato::is_container<yato::array_nd<int, 2, 2, 2, 2>>::value, "invalid is_container");
    static_assert(yato::is_container<std::vector<double>>::value, "invalid is_container");
    static_assert(yato::is_container<std::array<int, 4>>::value, "invalid is_container");
    static_assert(yato::is_container<std::list<short>>::value, "invalid is_container");
    static_assert(yato::is_container<std::map<std::string, int>>::value, "invalid is_container");
}

namespace
{

    template <typename Cy_>
    void test_read_1d(const Cy_& c, bool testing_continous)
    {
        using ops = yato::container_ops<Cy_>;

        EXPECT_EQ(static_cast<size_t>(4), ops::size(c));
        EXPECT_EQ(static_cast<size_t>(4), ops::size(c, 0));
        EXPECT_EQ(testing_continous, ops::continuous(c));

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

} // namespace

TEST(Yato_ContainerTraits, read_1d)
{
    std::array<int, 4> stdArr = {1, 2, 3, 4};
    std::vector<int> stdVec = {1, 2, 3, 4};
    std::list<int> stdList = {1, 2, 3, 4};
    yato::vector_1d<int> yatoVec = {1, 2, 3, 4};
    yato::array_nd<int, 4> yatoArr = {1, 2, 3, 4};
    yato::array_view_1d<int> yatoView = yato::view(yatoVec);
    yato::array_view_1d<const int> yatoCView = yato::cview(yatoVec);

    test_read_1d(stdArr, true);
    test_read_1d(stdVec, true);
    test_read_1d(stdList, false);
    test_read_1d(yatoVec, true);
    test_read_1d(yatoArr, true);
    test_read_1d(yatoView, true);
    test_read_1d(yatoCView, true);

    yato::vector_2d<int> yatoVec2d = { {1, 2, 3, 4}, {1, 1, 1, 1} };
    yato::array_nd<int, 2, 4> yatoArr2d = {{ {1, 2, 3, 4}, {1, 1, 1, 1} }};
    test_read_1d(yatoVec2d[0], true);
    test_read_1d(yatoArr2d[0], true);
}


TEST(Yato_ContainerND, access)
{
    yato::vector_nd<int, 2> v2(yato::dims(4, 4), -1);
    //test_write(v2);  // can't deduce :c
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
    //test_write(arr2);  // to be done
    test_write(v);

    //test_read(arr2);  // to be done
    test_read(v);

    EXPECT_EQ(42, arr2[0][0]);
    EXPECT_EQ(10, arr2[1][0]);
    EXPECT_EQ(10, arr2[2][0]);
    EXPECT_EQ(10, arr2[3][0]);
}

TEST(Yato_ContainerND, access_1)
{
    yato::vector_nd<int, 1> v1(yato::dims(8), -1);
    // test_write(v1);   // can't deduce :c
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
