#include "gtest/gtest.h"

#include "yato/container_nd.h"
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
    }

    template <typename Ty_, typename Impl_>
    void test_read(const yato::container_nd<Ty_, 2, Impl_> & src)
    {
        EXPECT_EQ(42, src[0][0]);
        for(auto it = std::next(src.cbegin()); it != src.cend(); ++it) {
            EXPECT_EQ(10, (*it)[0]);
        }
        EXPECT_EQ(20, *(src.cdata() + 1));
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
    }

    template <typename Ty_, typename Impl_>
    void test_read(const yato::container_nd<Ty_, 1, Impl_> & src)
    {
        EXPECT_EQ(42, src[0]);
        for(auto it = std::next(src.cbegin(), 2); it != src.cend(); ++it) {
            EXPECT_EQ(10, (*it));
        }
        EXPECT_EQ(20, *(src.cdata() + 1));
    }
}

TEST(Yato_ContainerND, access)
{
    yato::vector_nd<int, 2> v2(yato::dims(4, 4), -1);
    test_write(v2);
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

TEST(Yato_ContainerND, access_1)
{
    yato::vector_nd<int, 1> v1(yato::dims(8), -1);
    test_write(v1);
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
