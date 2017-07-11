#include "gtest/gtest.h"

#include <yato/actors/private/scheduler.h>

TEST(Yato_Actors, scheduler_1)
{
    yato::actors::scheduler scheduler;

    auto now = std::chrono::high_resolution_clock::now();

    scheduler.enqueue(now, [] { std::cout << "Task 1\n"; });
    scheduler.enqueue(now, [] { std::cout << "Task 2\n"; });
    scheduler.enqueue(now, [] { std::cout << "Task 3\n"; });
}


TEST(Yato_Actors, scheduler_2)
{

    yato::actors::scheduler scheduler;

    auto now = std::chrono::high_resolution_clock::now();

    scheduler.enqueue(now + std::chrono::milliseconds(10), [] { std::cout << "Task 1\n"; });
    scheduler.enqueue(now + std::chrono::milliseconds(20), [] { std::cout << "Task 2\n"; });
    scheduler.enqueue(now + std::chrono::milliseconds(30), [] { std::cout << "Task 3\n"; });
}

TEST(Yato_Actors, scheduler_3)
{

    yato::actors::scheduler scheduler;

    auto now = std::chrono::high_resolution_clock::now();

    scheduler.enqueue(now + std::chrono::milliseconds(30), [] { std::cout << "Task 1\n"; });
    scheduler.enqueue(now + std::chrono::milliseconds(20), [] { std::cout << "Task 2\n"; });
    scheduler.enqueue(now + std::chrono::milliseconds(10), [] { std::cout << "Task 3\n"; });
}


TEST(Yato_Actors, scheduler_4)
{
    constexpr uint32_t N = 100;
    std::vector<int> markers;
    std::mutex mutex;

    {
        yato::actors::scheduler scheduler;
        for (uint32_t i = 0; i < N; ++i) {
            int timeout = std::rand() % 100 + 1000;
            scheduler.enqueue(std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(timeout), [timeout, &markers, &mutex]{
                std::unique_lock<std::mutex> lock(mutex);
                markers.push_back(timeout);
            });
        }
    }
    for (int i = 1; i < markers.size(); ++i) {
        ASSERT_GE(markers[i], markers[i - 1]);
    }
}

