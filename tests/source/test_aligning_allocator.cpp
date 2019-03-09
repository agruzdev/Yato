/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2019 Alexey Gruzdev
 */

#include "gtest/gtest.h"

#include <vector>
#include <yato/aligning_allocator.h>
#include <yato/vector_nd.h>

namespace
{
    class Foo
    {
        char x[512]; 

        void foo() {
            (void)x;
        }

    public:
        Foo()  = default;
        ~Foo() = default;
    };

    template<typename T, size_t Align>
    void make_align_test()
    {
        yato::aligning_allocator<T, Align> alloc;

        size_t sizes[] = { 1, 20, 10, 2, 3, 90, 199, 12, 13, 14, 17, 7, 9, 1000 };

        std::vector<T*> pointers;
        for (size_t n : sizes) {
            T* ptr = alloc.allocate(n);
            EXPECT_NE(nullptr, ptr);
            EXPECT_EQ(0U, reinterpret_cast<size_t>(ptr) % Align);
            pointers.push_back(ptr);
        }

        for (size_t i = pointers.size(); i > 0; --i) {
            alloc.deallocate(pointers[i - 1], sizes[i - 1]);
        }

        const int ITERS = 10000;
        for (int i = 0; i < ITERS; ++i) {
            T* ptr = alloc.allocate(1);
            EXPECT_NE(nullptr, ptr);
            EXPECT_EQ(0U, reinterpret_cast<size_t>(ptr) % Align);
            std::allocator_traits<decltype(alloc)>::construct(alloc, ptr);
            std::allocator_traits<decltype(alloc)>::destroy(alloc, ptr);
            alloc.deallocate(ptr, 1);
        }
    }

    template<typename T, size_t Align>
    void make_vector_test()
    {
        const int ITERS = 1000;
        for (int i = 0; i < 2; ++i) {
            std::vector<T, yato::aligning_allocator<T, Align>> v(10, 1);
            EXPECT_EQ(static_cast<T>(1), v[0]);
            EXPECT_EQ(0U, reinterpret_cast<size_t>(&v[0]) % Align);
            v.resize(1);
            EXPECT_EQ(static_cast<T>(1), v[0]);
            EXPECT_EQ(0U, reinterpret_cast<size_t>(&v[0]) % Align);
            v.resize(100);
            EXPECT_EQ(0U, reinterpret_cast<size_t>(&v[0]) % Align);
            v.clear();
            v.shrink_to_fit();
            for (int j = 0; j < ITERS; ++j) {
                v.push_back(2);
                EXPECT_EQ(static_cast<T>(2), v.back());
                EXPECT_EQ(0U, reinterpret_cast<size_t>(&v[0]) % Align);
            }
        }
    }

    template<typename T, size_t Align>
    void make_vector_2d_test()
    {
        const int ITERS = 1000;
        for (int i = 0; i < 2; ++i) {
            yato::vector_2d<T, yato::aligning_allocator<T, Align>> v(yato::dims(10, 10), 1);
            EXPECT_EQ(static_cast<T>(1), v[0][0]);
            EXPECT_EQ(0U, reinterpret_cast<size_t>(v.data()) % Align);
            v.resize(yato::dims(1, 1));
            EXPECT_EQ(static_cast<T>(1), v[0][0]);
            EXPECT_EQ(0U, reinterpret_cast<size_t>(v.data()) % Align);
            v.resize(yato::dims(32, 32));
            EXPECT_EQ(0U, reinterpret_cast<size_t>(v.data()) % Align);
            auto v1(std::move(v).reshape(yato::dims(32*32)));
            EXPECT_EQ(0U, reinterpret_cast<size_t>(v1.data()) % Align);
            v1.clear();
            v1.shrink_to_fit();
            for (int j = 0; j < ITERS; ++j) {
                v1.push_back(2);
                EXPECT_EQ(static_cast<T>(2), v1.back());
                EXPECT_EQ(0U, reinterpret_cast<size_t>(v1.data()) % Align);
            }
        }
    }
}

TEST(Yato_AlignAlloc, common)
{
    yato::aligning_allocator<int, 3> a;
    auto* p = a.allocate(4);
    a.deallocate(p, 4);
    size_t overhead = decltype(a)::extra_bytes;
    EXPECT_EQ(3U, overhead);

    make_align_test<char, 1>();
    make_align_test<char, 2>();
    make_align_test<char, 3>(); //not power of two
    make_align_test<char, 4>();
    make_align_test<char, 7>(); // not power of two
    make_align_test<char, 8>();
    make_align_test<char, 121>(); // bigger than max_align_t
    make_align_test<char, 128>();
    make_align_test<char, 1024>();

    make_align_test<int, 1>();
    make_align_test<int, 2>();
    make_align_test<int, 3>();
    make_align_test<int, 4>();
    make_align_test<int, 7>();
    make_align_test<int, 8>();
    make_align_test<int, 121>();
    make_align_test<int, 128>();
    make_align_test<int, 1024>();

    make_align_test<long double, 1>();
    make_align_test<long double, 2>();
    make_align_test<long double, 3>();
    make_align_test<long double, 4>();
    make_align_test<long double, 7>();
    make_align_test<long double, 8>();
    make_align_test<long double, 121>();
    make_align_test<long double, 128>();
    make_align_test<long double, 1024>();

    make_align_test<Foo, 1>();
    make_align_test<Foo, 2>();
    make_align_test<Foo, 3>();
    make_align_test<Foo, 4>();
    make_align_test<Foo, 7>();
    make_align_test<Foo, 8>();
    make_align_test<Foo, 121>();
    make_align_test<Foo, 128>();
    make_align_test<Foo, 1024>();
}

TEST(Yato_AlignAlloc, vector)
{
    make_vector_test<char, 1>();
    make_vector_test<char, 2>();
    make_vector_test<char, 4>();
    make_vector_test<char, 8>();
    
    make_vector_test<int, 1>();
    make_vector_test<int, 2>();
    make_vector_test<int, 4>();
    make_vector_test<int, 8>();
    
    make_vector_test<long double, 1>();
    make_vector_test<long double, 2>();
    make_vector_test<long double, 4>();
    make_vector_test<long double, 8>();
}


TEST(Yato_AlignAlloc, vector_2d)
{
    make_vector_2d_test<char, 1>();
    make_vector_2d_test<char, 2>();
    make_vector_2d_test<char, 4>();
    make_vector_2d_test<char, 8>();

    make_vector_2d_test<int, 1>();
    make_vector_2d_test<int, 2>();
    make_vector_2d_test<int, 4>();
    make_vector_2d_test<int, 8>();

    make_vector_2d_test<long double, 1>();
    make_vector_2d_test<long double, 2>();
    make_vector_2d_test<long double, 4>();
    make_vector_2d_test<long double, 8>();
}

