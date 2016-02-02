#include "gtest/gtest.h"

#include <memory>
#include <initializer_list>
#include <yato/vector_nd.h>


TEST(Yato_VectorND, ctor)
{
    class A {
        int m_val;
    public:
        A(int x): m_val(x) {};
    };

    try {
        yato::vector_nd<int, 3> vec0{};
        yato::vector_nd<int, 2> vec1({ 2, 3 });
        yato::vector_nd<int, 1> vec2({ 5 }, 1);
        yato::vector_nd<float, 3> vec3({ 2, 3, 3 }, 11.0f);

        yato::vector_nd<int, 2> vec4 = { {1, 1, 1}, {2, 2, 2} };
        yato::vector_nd<int, 3> vec5 = { { {1, 1}, {1, 2}, {1, 3} }, { {2, 4}, {2, 5}, {2, 6} } };
        yato::vector_nd<int, 2> vec6 = { };

        yato::vector_nd<A, 2> vec7 = { { A(1) } };
    }
    catch (...)
    {
        EXPECT_TRUE(false);
    }
    EXPECT_TRUE(true);
}


