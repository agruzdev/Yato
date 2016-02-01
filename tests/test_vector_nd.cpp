#include "gtest/gtest.h"

#include <memory>
#include <initializer_list>
#include <yato/vector_nd.h>


TEST(Yato_VectorND, ctor)
{
    try {
        yato::vector_nd<int, 3> vec0{};
        yato::vector_nd<int, 2> vec1({ 2, 3 });
        yato::vector_nd<int, 1> vec2({ 5 }, 1);
        yato::vector_nd<float, 3> vec3({ 2, 3, 3 }, 11.0f);
    }
    catch (...)
    {
        EXPECT_TRUE(false);
    }
    EXPECT_TRUE(true);
}


