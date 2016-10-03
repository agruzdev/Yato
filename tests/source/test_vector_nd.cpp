#include "gtest/gtest.h"

#include <memory>
#include <algorithm>
#include <numeric>
#include <initializer_list>
#include <yato/vector_nd.h>


TEST(Yato_VectorND, common)
{
    class A {
        int m_val;
    public:
        A(int x): m_val(x) {};
    };

    try {
        yato::vector_nd<int, 3> vec0{};
        EXPECT_TRUE(vec0.empty());
        EXPECT_EQ(3, vec0.dimensions_num());
        EXPECT_EQ(0, vec0.size(0));
        EXPECT_EQ(0, vec0.size(1));
        EXPECT_EQ(0, vec0.size(2));
        EXPECT_EQ(0, vec0.total_size());

        yato::vector_nd<int, 2> vec1(yato::dims(2, 3));
        EXPECT_FALSE(vec1.empty());
        EXPECT_EQ(2, vec1.dimensions_num());
        EXPECT_EQ(2, vec1.size(0));
        EXPECT_EQ(3, vec1.size(1));
        EXPECT_EQ(6, vec1.total_size());

        EXPECT_EQ(2, yato::height_2d(vec1));
        EXPECT_EQ(3, yato::width_2d(vec1));

        yato::vector_nd<int, 1> vec2(yato::dims(5), 1);
        EXPECT_EQ(5, vec2.size(0));

        yato::vector_nd<float, 3> vec3(yato::dims(2, 3, 3), 11.0f);

        yato::vector_nd<int, 2> vec4 = { {1, 1, 1}, {2, 2, 2} };
        yato::vector_nd<int, 3> vec5 = { { {1, 1}, {1, 2}, {1, 3} }, { {2, 4}, {2, 5}, {2, 6} } };
        yato::vector_nd<int, 2> vec6 = { };

        EXPECT_EQ(2, yato::depth_3d(vec5));
        EXPECT_EQ(3, yato::height_3d(vec5));
        EXPECT_EQ(2, yato::width_3d(vec5));

        yato::vector_nd<A, 2> vec7 = { { A(1) } };

        std::vector<int> sizes;
        sizes.push_back(1);
        sizes.push_back(5);
        sizes.push_back(2);
        const yato::vector_nd<int, 3> vec8(yato::make_range(sizes.cbegin(), sizes.cend()), 1);

        yato::vector_nd<float, 3> vec9(yato::dims(2, 0, 3), 1.0f);
        EXPECT_TRUE(vec9.empty());
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

TEST(Yato_VectorND, access_2)
{
    yato::vector_nd<int, 3> vec5 = { { { 1, 1 },{ 1, 2 },{ 1, 3 } },{ { 2, 4 },{ 2, 5 },{ 2, 6 } } };
    EXPECT_EQ(2, vec5[0].dimensions_num());
    EXPECT_EQ(3, vec5[0].size(0));
    EXPECT_EQ(2, vec5[0].size(1));
    EXPECT_EQ(6, vec5[0].total_size());

    auto vec0 = yato::vector_nd<int, 3>(yato::dims(2, 0, 2), 1);
    EXPECT_EQ(3, vec0.dimensions_num());
    EXPECT_EQ(2, vec0[0].dimensions_num());
    EXPECT_EQ(0, vec0.total_size());
    EXPECT_EQ(0, vec0[0].total_size());
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

TEST(Yato_VectorND, assign)
{
    yato::vector_nd<int, 3> vec = {};
    
    EXPECT_TRUE(vec.empty());
    for (const int & x : vec.plain_crange()) {
        (void)x;
        EXPECT_TRUE(false);
    }

    vec.assign(yato::dims(2, 2, 2), 1);
    
    EXPECT_FALSE(vec.empty());
    for (const int & x : vec.plain_crange()) {
        EXPECT_EQ(1, x);
    }
}

TEST(Yato_VectorND, proxy_iter)
{
    yato::vector_nd<int, 2> vec5 = { { 1, 1 },{ 1, 2 },{ 1, 3 } };

    auto row = vec5.cbegin();
    auto end = vec5.cend();
    for (int i = 1; row != end; ++row, ++i) {
        const int & x = row[0];
        EXPECT_EQ(1, x);
        EXPECT_EQ(row[1], i);
    }

    yato::vector_nd<int, 2> vec6 = { { 1, 1, 1 },{ 0, 0, 0 },{ 1, 1, 1 } };
    auto row_2 = std::next(vec6.begin());
    std::fill(std::begin(row_2), std::end(row_2), 1);
    for (auto x : vec6.plain_range()) {
        EXPECT_EQ(1, x);
    }
}


TEST(Yato_VectorND, copy_proxy)
{
    yato::vector_nd<int, 3> vec5 = { { { 1, 1 },{ 1, 2 },{ 1, 3 } },{ { 2, 4 },{ 2, 5 },{ 2, 6 } } };

    yato::vector_nd<long, 2> copy1 = {};
    copy1 = vec5[1];
    EXPECT_EQ(2, copy1[0][0]);
    EXPECT_EQ(4, copy1[0][1]);
    EXPECT_EQ(2, copy1[1][0]);
    EXPECT_EQ(5, copy1[1][1]);
    EXPECT_EQ(2, copy1[2][0]);
    EXPECT_EQ(6, copy1[2][1]);

    yato::vector_nd<long long, 1> copy2{ copy1[1] };
    EXPECT_EQ(2, copy2[0]);
    EXPECT_EQ(5, copy2[1]);
}

TEST(Yato_VectorND, push_pop)
{
    yato::vector_nd<short, 1> vec1d_1 = { 1, 2 };
    yato::vector_nd<uint8_t, 1> vec1d_2 = { 3, 4 };

    yato::vector_nd<int, 2>  vec2d = { {1, 2}, {3, 4} };
    vec2d.clear();
    EXPECT_TRUE(vec2d.empty());

    vec2d.push_back(std::move(vec1d_1));
    vec2d.push_back(std::move(vec1d_2));

    yato::vector_nd<long, 3> vec3d = {};

    vec3d.push_back(vec2d);
    EXPECT_EQ(1, vec3d.size(0));
    EXPECT_EQ(2, vec3d.size(1));
    EXPECT_EQ(2, vec3d.size(2));

    vec3d.push_back(vec2d);
    EXPECT_EQ(2, vec3d.size(0));
    EXPECT_EQ(2, vec3d.size(1));
    EXPECT_EQ(2, vec3d.size(2));

    vec3d.push_back(vec2d);
    EXPECT_EQ(3, vec3d.size(0));
    EXPECT_EQ(2, vec3d.size(1));
    EXPECT_EQ(2, vec3d.size(2));

    vec3d.pop_back();
    EXPECT_EQ(2, vec3d.size(0));
    EXPECT_EQ(2, vec3d.size(1));
    EXPECT_EQ(2, vec3d.size(2));
    
    vec3d.pop_back();
    EXPECT_EQ(1, vec3d.size(0));
    EXPECT_EQ(2, vec3d.size(1));
    EXPECT_EQ(2, vec3d.size(2));

    vec3d.pop_back();
    EXPECT_TRUE(vec3d.empty());
}


TEST(Yato_VectorND, vec_1D)
{
    std::vector<int> std_vec1 = { 1, 2, 3 };
    std::vector<int> std_vec2 = { 4, 5, 6 };

    yato::vector_nd<int, 1> vec1 = std_vec1;
    EXPECT_EQ(3, vec1.size(0));
    EXPECT_EQ(1, vec1[0]);
    EXPECT_EQ(2, vec1[1]);
    EXPECT_EQ(3, vec1[2]);

    yato::vector_nd<int, 1> vec2 = std::move(std_vec2);
    EXPECT_EQ(3, vec2.size(0));
    EXPECT_EQ(4, vec2[0]);
    EXPECT_EQ(5, vec2[1]);
    EXPECT_EQ(6, vec2[2]);

    vec1 = std::move(vec2);
    EXPECT_EQ(3, vec1.size(0));
    EXPECT_EQ(4, vec1[0]);
    EXPECT_EQ(5, vec1[1]);
    EXPECT_EQ(6, vec1[2]);

    std_vec1 = vec1;
    EXPECT_EQ(3, std_vec1.size());
    EXPECT_EQ(4, std_vec1[0]);
    EXPECT_EQ(5, std_vec1[1]);
    EXPECT_EQ(6, std_vec1[2]);

    std_vec2 = std::move(vec1);
    EXPECT_EQ(3, std_vec2.size());
    EXPECT_EQ(4, std_vec2[0]);
    EXPECT_EQ(5, std_vec2[1]);
    EXPECT_EQ(6, std_vec2[2]);
}

TEST(Yato_VectorND, insert)
{
    yato::vector_nd<int, 2> vec = { { 1, 1 },{ 3, 3 } };

    yato::vector_nd<short, 1> line = { 2, 2 };
    vec.insert(std::next(vec.begin()), line);

    vec.insert(vec.begin(), yato::vector_nd<int, 1>{ 0, 0 } );
    vec.insert(vec.end(), yato::vector_nd<int, 1>{ 4, 4 });

    int i = 0;
    for (const auto & row : vec) {
        EXPECT_EQ(i, row[0]);
        EXPECT_EQ(i, row[1]);
        ++i;
    }
}

TEST(Yato_VectorND, insert_count)
{
    yato::vector_nd<int, 2> vec = { { 1, 1 },{ 3, 3 } };

    yato::vector_nd<short, 1> line = { 2, 2 };
    vec.insert(std::next(vec.begin()), 10, line);

    int i = 0;
    for (const auto & row : vec) {
        if (i > 0 && i < 11) {
            EXPECT_EQ(2, row[0]);
            EXPECT_EQ(2, row[1]);
        }
        ++i;
    }
}


TEST(Yato_VectorND, insert_range)
{
    yato::vector_nd<int, 2> vec1 = { { 1, 1 },{ 4, 4 } };
    yato::vector_nd<int, 2> vec2 = { { 2, 2 },{ 3, 3 } };

    vec1.insert(std::next(vec1.begin()), vec2.begin(), vec2.end());

    int i = 1;
    for (const auto & row : vec1) {
        EXPECT_EQ(i, row[0]);
        EXPECT_EQ(i, row[1]);
        ++i;
    }
}


TEST(Yato_VectorND, erase)
{
    yato::vector_nd<int, 2> vec1 = { { 1, 1 },{ 4, 4 }, { 5, 5 }, { 2, 2 }, { 3, 3 } };

    auto it = vec1.erase(vec1.begin() + 1, vec1.begin() + 3);
    EXPECT_EQ(3, vec1.size(0));
    EXPECT_EQ(2, (*it)[0]);
    EXPECT_EQ(2, (*it)[1]);

    it = vec1.erase(it - 1);
    EXPECT_EQ(2, vec1.size(0));
    EXPECT_EQ(2, (*it)[0]);
    EXPECT_EQ(2, (*it)[1]);
}

