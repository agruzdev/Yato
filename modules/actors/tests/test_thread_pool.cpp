/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2020 Alexey Gruzdev
 */

#include "gtest/gtest.h"

#include <atomic>

#include <yato/actors/private/thread_pool.h>

TEST(Yato_Actors, thread_pool_1)
{

    yato::actors::thread_pool tpool(1);

    tpool.enqueue([] { std::cout << "Task 1\n"; });
    tpool.enqueue([] { std::cout << "Task 2\n"; });
    tpool.enqueue([] { std::cout << "Task 3\n"; });

}

TEST(Yato_Actors, thread_pool_2)
{

    yato::actors::thread_pool tpool(4);

    tpool.enqueue([] { std::cout << "Task 1\n"; });
    tpool.enqueue([] { std::cout << "Task 2\n"; });
    tpool.enqueue([] { std::cout << "Task 3\n"; });

}

TEST(Yato_Actors, thread_pool_3)
{
    constexpr uint32_t N = 4096;

    std::atomic<uint32_t> counter{ 0 };

    {
        yato::actors::thread_pool tpool(8);
        for (uint32_t i = 0; i < N; ++i) {
            tpool.enqueue([&counter] { ++counter; });
        }
    }

    ASSERT_EQ(counter.load(), N);
}

