#include "gtest/gtest.h"

#include <memory>
#include <algorithm>
#include <numeric>
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

        std::vector<int> sizes;
        sizes.push_back(1);
        sizes.push_back(5);
        sizes.push_back(2);
        const yato::vector_nd<int, 3> vec8(yato::make_range(sizes.cbegin(), sizes.cend()), 1);
    }
    catch (...)
    {
        EXPECT_TRUE(false);
    }
    EXPECT_TRUE(true);
}



TEST(Yato_VectorND, access)
{
    yato::vector_nd<int, 2> vec1 = { {1, 2}, {3, 4} };
    EXPECT_TRUE(vec1[0][0] == 1);
    EXPECT_TRUE(vec1[0][1] == 2);
    EXPECT_TRUE(vec1[1][0] == 3);
    EXPECT_TRUE(vec1[1][1] == 4);
}

TEST(Yato_VectorND, access_1)
{
    std::array<uint8_t, 3> dims;
    std::generate(dims.begin(), dims.end(), []() { return static_cast<uint8_t>(std::rand() % 101); });

    float val = (std::rand() % 1000) / 10.0f;
    yato::vector_nd<float, 3> vec_nd(yato::make_range(dims.begin(), dims.end()), val);

    for (size_t i = 0; i < dims[0]; ++i) {
        for (size_t j = 0; j < dims[1]; ++j) {
            for (size_t k = 0; k < dims[2]; ++k) {
                float x;
                EXPECT_NO_THROW(x = vec_nd[i][j][k]);
                EXPECT_EQ(val, x);
            }
        }
    }
}

TEST(Yato_VectorND, operator_at)
{
    yato::vector_nd<int, 2> vec1 = { { 1, 2 },{ 3, 4 } };
    EXPECT_TRUE(vec1.at(0, 0) == 1);
    EXPECT_TRUE(vec1.at(0, 1) == 2);
    EXPECT_TRUE(vec1.at(1, 0) == 3);
    EXPECT_TRUE(vec1.at(1, 1) == 4);
    
    EXPECT_THROW(vec1.at(0, 3), yato::assertion_error);
    EXPECT_THROW(vec1.at(3, 1), yato::assertion_error);
    EXPECT_THROW(vec1.at(1, 2), yato::assertion_error);
    EXPECT_THROW(vec1.at(2, 1), yato::assertion_error);
}

TEST(Yato_VectorND, operator_at_1)
{
    std::array<uint8_t, 3> dims;
    std::generate(dims.begin(), dims.end(), []() { return static_cast<uint8_t>(std::rand() % 51); });

    float val = (std::rand() % 1000) / 10.0f;
    yato::vector_nd<float, 3> vec_nd(yato::make_range(dims.begin(), dims.end()), val);

    for (size_t i = 0; i < dims[0]; ++i) {
        for (size_t j = 0; j < dims[1]; ++j) {
            for (size_t k = 0; k < dims[2]; ++k) {
                float x;
                EXPECT_NO_THROW(x = vec_nd.at(i, j, k));
                EXPECT_THROW(vec_nd.at(dims[0] + i, dims[1] + j, dims[2] + k), yato::assertion_error);
                EXPECT_EQ(val, x);
            }
        }
    }
}