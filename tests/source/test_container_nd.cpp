/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2020 Alexey Gruzdev
 */

#include "gtest/gtest.h"

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


TEST(Yato_ContainerND, sampler_default)
{
    yato::vector_nd<int, 2> v1 = {
        { 1, 2 },
        { 3, 4 }
    };

    EXPECT_EQ(1, v1.at<yato::sampler_default>(0, 0));
    EXPECT_EQ(2, v1.at<yato::sampler_default>(0, 1));
    EXPECT_EQ(3, v1.at<yato::sampler_default>(1, 0));
    EXPECT_EQ(4, v1.at<yato::sampler_default>(1, 1));

    EXPECT_THROW(v1.at<yato::sampler_default>(1, 2), yato::out_of_range_error);
    EXPECT_THROW(v1.at<yato::sampler_default>(2, 1), yato::out_of_range_error);
    EXPECT_THROW(v1.at<yato::sampler_default>(10, 1), yato::out_of_range_error);
    EXPECT_THROW(v1.at<yato::sampler_default>(10, 11), yato::out_of_range_error);
}

TEST(Yato_ContainerND, sampler_no_check)
{
    yato::vector_nd<int, 2> v1 = {
        { 1, 2 },
        { 3, 4 }
    };

    EXPECT_EQ(1, v1.at<yato::sampler_no_check>(0, 0));
    EXPECT_EQ(2, v1.at<yato::sampler_no_check>(0, 1));
    EXPECT_EQ(3, v1.at<yato::sampler_no_check>(1, 0));
    EXPECT_EQ(4, v1.at<yato::sampler_no_check>(1, 1));
}

TEST(Yato_ContainerND, sampler_zero)
{
    yato::vector_nd<int, 2> v1 = {
        { 1, 2 },
        { 3, 4 }
    };

    EXPECT_EQ(1, v1.at<yato::sampler_zero>(0, 0));
    EXPECT_EQ(2, v1.at<yato::sampler_zero>(0, 1));
    EXPECT_EQ(3, v1.at<yato::sampler_zero>(1, 0));
    EXPECT_EQ(4, v1.at<yato::sampler_zero>(1, 1));

    EXPECT_EQ(0, v1.at<yato::sampler_zero>(-1, 0));
    EXPECT_EQ(0, v1.at<yato::sampler_zero>(0, -1));
    EXPECT_EQ(0, v1.at<yato::sampler_zero>(1, 2));
    EXPECT_EQ(0, v1.at<yato::sampler_zero>(2, 1));
}

TEST(Yato_ContainerND, sampler_clamp)
{
    yato::vector_nd<int, 2> v1 = {
        { 1, 2 },
        { 3, 4 }
    };

    EXPECT_EQ(1, v1.at<yato::sampler_clamp>(0, 0));
    EXPECT_EQ(2, v1.at<yato::sampler_clamp>(0, 1));
    EXPECT_EQ(3, v1.at<yato::sampler_clamp>(1, 0));
    EXPECT_EQ(4, v1.at<yato::sampler_clamp>(1, 1));

    EXPECT_EQ(1, v1.at<yato::sampler_clamp>(-1, 0));
    EXPECT_EQ(1, v1.at<yato::sampler_clamp>(0, -1));
    EXPECT_EQ(4, v1.at<yato::sampler_clamp>(1, 2));
    EXPECT_EQ(4, v1.at<yato::sampler_clamp>(2, 1));


    yato::vector_nd<int, 1> v2 = {
        { 1, 2, 3 }
    };

    auto view2 = yato::cview(v2).reshape(yato::dims(1, 1, 3));

    EXPECT_EQ(1, view2.at<yato::sampler_clamp>(0, 0, 0));
    EXPECT_EQ(2, view2.at<yato::sampler_clamp>(0, 0, 1));
    EXPECT_EQ(3, view2.at<yato::sampler_clamp>(0, 0, 2));
    EXPECT_EQ(1, view2.at<yato::sampler_clamp>(1, 0, 0));
    EXPECT_EQ(2, view2.at<yato::sampler_clamp>(0, 2, 1));
    EXPECT_EQ(3, view2.at<yato::sampler_clamp>(3, 0, 2));
    EXPECT_EQ(1, view2.at<yato::sampler_clamp>(3, 2, 0));
    EXPECT_EQ(2, view2.at<yato::sampler_clamp>(1, 1, 1));
    EXPECT_EQ(3, view2.at<yato::sampler_clamp>(4, 4, 2));
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

TEST(Yato_ContainerND, custom_sampler)
{
    yato::vector_nd<int, 2> v1 = {
        { 1, 2 },
        { 3, 4 }
    };

    sampler_oob_counter sampler{};

    EXPECT_EQ(1, v1.fetch(sampler, 0, 0));
    EXPECT_EQ(2, v1.fetch(sampler, 0, 1));
    EXPECT_EQ(3, v1.fetch(sampler, 1, 0));
    EXPECT_EQ(4, v1.fetch(sampler, 1, 1));

    EXPECT_EQ(0, v1.fetch(sampler, -1, 0));
    EXPECT_EQ(0, v1.fetch(sampler, 0, -1));
    EXPECT_EQ(0, v1.fetch(sampler, 1, 2));
    EXPECT_EQ(0, v1.fetch(sampler, 2, 1));

    EXPECT_EQ(4, sampler.count);
}
