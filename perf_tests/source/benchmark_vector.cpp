/**
* YATO library
*
* Apache License, Version 2.0
* Copyright (c) 2016-2020 Alexey Gruzdev
*/

#include <algorithm>
#include <cmath>
#include <numeric>

#include <yato/vector_nd.h>

#include <benchmark/benchmark.h>


void Vector1D_Write_STL(benchmark::State& state)
{
    static const int RND_SEED = 3;
    std::srand(RND_SEED);

    const int32_t N = yato::narrow_cast<int32_t>(state.range(0));

    std::vector<int32_t> vec(10);
    std::generate(vec.begin(), vec.end(), std::rand);

    const size_t i = yato::narrow_cast<size_t>(std::rand() % 2);

    for (auto _ : state) {
        for (int32_t n = 0; n < N; ++n) {
            vec[i + (n & 0x3)] = n;
        }
        benchmark::DoNotOptimize(std::accumulate(vec.cbegin(), vec.cend(), 0));
    }
}
BENCHMARK(Vector1D_Write_STL)->Arg(1000)->Arg(10000)->Arg(100000)->Arg(1000000);


void Vector1D_Write_Yato(benchmark::State& state)
{
    static const int RND_SEED = 3;
    std::srand(RND_SEED);

    const int32_t N = yato::narrow_cast<int32_t>(state.range(0));

    yato::vector_1d<int32_t> vec(10);
    std::generate(vec.begin(), vec.end(), std::rand);

    const size_t i = yato::narrow_cast<size_t>(std::rand() % 2);

    for (auto _ : state) {
        for (int32_t n = 0; n < N; ++n) {
            vec[i + (n & 0x3)] = n;
        }
        benchmark::DoNotOptimize(std::accumulate(vec.cbegin(), vec.cend(), 0));
    }
}
BENCHMARK(Vector1D_Write_Yato)->Arg(1000)->Arg(10000)->Arg(100000)->Arg(1000000);


void Vector2D_Write_STL(benchmark::State& state)
{
    static const int RND_SEED = 3;
    std::srand(RND_SEED);

    const int32_t N = yato::narrow_cast<int32_t>(state.range(0));

    const size_t S0 = yato::narrow_cast<size_t>(10 + std::rand() % 2);
    const size_t S1 = yato::narrow_cast<size_t>(10 + std::rand() % 2);

    std::vector<int32_t> vec(S0 * S1);
    std::generate(vec.begin(), vec.end(), std::rand);

    const size_t i0 = yato::narrow_cast<size_t>(std::rand() % 2);
    const size_t i1 = yato::narrow_cast<size_t>(std::rand() % 2);

    for (auto _ : state) {
        for (int32_t n = 0; n < N; ++n) {
            const size_t j0 = i0 + (n & 0x1);
            const size_t j1 = i1 + (n & 0x3);
            vec[j0 * S1 + j1] = n;
        }
        benchmark::DoNotOptimize(std::accumulate(vec.cbegin(), vec.cend(), 0));
    }
}
BENCHMARK(Vector2D_Write_STL)->Arg(1000)->Arg(10000)->Arg(100000)->Arg(1000000);


void Vector2D_Write_Yato(benchmark::State& state)
{
    static const int RND_SEED = 3;
    std::srand(RND_SEED);

    const int32_t N = yato::narrow_cast<int32_t>(state.range(0));

    const size_t S0 = yato::narrow_cast<size_t>(10 + std::rand() % 2);
    const size_t S1 = yato::narrow_cast<size_t>(10 + std::rand() % 2);

    yato::vector_2d<int32_t> vec(yato::dims(S0, S1));
    std::generate(vec.plain_begin(), vec.plain_end(), std::rand);

    const size_t i0 = yato::narrow_cast<size_t>(std::rand() % 2);
    const size_t i1 = yato::narrow_cast<size_t>(std::rand() % 2);

    for (auto _ : state) {
        for (int32_t n = 0; n < N; ++n) {
            const size_t j0 = i0 + (n & 0x1);
            const size_t j1 = i1 + (n & 0x3);
            vec[j0][j1] = n;
        }
        benchmark::DoNotOptimize(std::accumulate(vec.plain_cbegin(), vec.plain_cend(), 0));
    }
}
BENCHMARK(Vector2D_Write_Yato)->Arg(1000)->Arg(10000)->Arg(100000)->Arg(1000000);



void Vector3D_Write_STL(benchmark::State& state)
{
    static const int RND_SEED = 3;
    std::srand(RND_SEED);

    const int32_t N = yato::narrow_cast<int32_t>(state.range(0));

    const size_t S0 = yato::narrow_cast<size_t>(10 + std::rand() % 2);
    const size_t S1 = yato::narrow_cast<size_t>(10 + std::rand() % 2);
    const size_t S2 = yato::narrow_cast<size_t>(10 + std::rand() % 2);

    std::vector<int32_t> vec(S0 * S1 * S2);
    std::generate(vec.begin(), vec.end(), std::rand);

    const size_t i0 = yato::narrow_cast<size_t>(std::rand() % 2);
    const size_t i1 = yato::narrow_cast<size_t>(std::rand() % 2);
    const size_t i2 = yato::narrow_cast<size_t>(std::rand() % 2);

    for (auto _ : state) {
        for (int32_t n = 0; n < N; ++n) {
            const size_t j0 = i0 + (n & 0x1);
            const size_t j1 = i1 + (n & 0x1);
            const size_t j2 = i2 + (n & 0x3);
            vec[(j0 * S1 + j1) * S2 + j2] = n;
        }
        benchmark::DoNotOptimize(std::accumulate(vec.cbegin(), vec.cend(), 0));
    }
}
BENCHMARK(Vector3D_Write_STL)->Arg(1000)->Arg(10000)->Arg(100000)->Arg(1000000);


void Vector3D_Write_Yato(benchmark::State& state)
{
    static const int RND_SEED = 3;
    std::srand(RND_SEED);

    const int32_t N = yato::narrow_cast<int32_t>(state.range(0));

    const size_t S0 = yato::narrow_cast<size_t>(10 + std::rand() % 2);
    const size_t S1 = yato::narrow_cast<size_t>(10 + std::rand() % 2);
    const size_t S2 = yato::narrow_cast<size_t>(10 + std::rand() % 2);

    yato::vector_3d<int32_t> vec(yato::dims(S0, S1, S2));
    std::generate(vec.plain_begin(), vec.plain_end(), std::rand);

    const size_t i0 = yato::narrow_cast<size_t>(std::rand() % 2);
    const size_t i1 = yato::narrow_cast<size_t>(std::rand() % 2);
    const size_t i2 = yato::narrow_cast<size_t>(std::rand() % 2);

    for (auto _ : state) {
        for (int32_t n = 0; n < N; ++n) {
            const size_t j0 = i0 + (n & 0x1);
            const size_t j1 = i1 + (n & 0x1);
            const size_t j2 = i2 + (n & 0x3);
            vec[j0][j1][j2] = n;
        }
        benchmark::DoNotOptimize(std::accumulate(vec.plain_cbegin(), vec.plain_cend(), 0));
    }
}
BENCHMARK(Vector3D_Write_Yato)->Arg(1000)->Arg(10000)->Arg(100000)->Arg(1000000);



void Vector1D_RndRead_STL(benchmark::State& state)
{
    static const int RND_SEED = 3;
    std::srand(RND_SEED);

    const size_t N = yato::narrow_cast<size_t>(state.range(0));

    const size_t S0 = yato::narrow_cast<size_t>(100 + std::rand() % 2);
    std::vector<int32_t> vec(S0);
    std::generate(vec.begin(), vec.end(), std::rand);

    std::vector<size_t> indexes(N);
    std::generate(indexes.begin(), indexes.end(), [S0]() {return yato::narrow_cast<size_t>(std::rand() % S0); });

    for (auto _ : state) {
        uint64_t sum = 0;
        for (size_t n = 0; n < N; ++n) {
            sum += vec[indexes[n]];
        }
        benchmark::DoNotOptimize(sum);
    }
}
BENCHMARK(Vector1D_RndRead_STL)->Arg(1000)->Arg(10000)->Arg(100000)->Arg(1000000);


void Vector1D_RndRead_Yato(benchmark::State& state)
{
    static const int RND_SEED = 3;
    std::srand(RND_SEED);

    const size_t N = yato::narrow_cast<size_t>(state.range(0));

    const size_t S0 = yato::narrow_cast<size_t>(100 + std::rand() % 2);
    yato::vector_1d<int32_t> vec(S0);
    std::generate(vec.begin(), vec.end(), std::rand);

    std::vector<size_t> indexes(N);
    std::generate(indexes.begin(), indexes.end(), [S0]() {return yato::narrow_cast<size_t>(std::rand() % S0); });

    for (auto _ : state) {
        uint64_t sum = 0;
        for (size_t n = 0; n < N; ++n) {
            sum += vec[indexes[n]];
        }
        benchmark::DoNotOptimize(sum);
    }
}
BENCHMARK(Vector1D_RndRead_Yato)->Arg(1000)->Arg(10000)->Arg(100000)->Arg(1000000);


void Vector2D_RndRead_STL(benchmark::State& state)
{
    static const int RND_SEED = 3;
    std::srand(RND_SEED);

    const size_t N = yato::narrow_cast<size_t>(state.range(0));

    const size_t S0 = yato::narrow_cast<size_t>(10 + std::rand() % 2);
    const size_t S1 = yato::narrow_cast<size_t>(10 + std::rand() % 2);
    std::vector<int32_t> vec(S0 * S1);
    std::generate(vec.begin(), vec.end(), std::rand);

    std::vector<std::tuple<size_t, size_t>> indexes(N);
    std::generate(indexes.begin(), indexes.end(), [S0, S1]() { return std::make_tuple(
        yato::narrow_cast<size_t>(std::rand() % S0),
        yato::narrow_cast<size_t>(std::rand() % S1));
        });

    for (auto _ : state) {
        uint64_t sum = 0;
        for (size_t n = 0; n < N; ++n) {
            size_t i0, i1;
            std::tie(i0, i1) = indexes[n];
            sum += vec[i0 * S1 + i1];
        }
        benchmark::DoNotOptimize(sum);
    }
}
BENCHMARK(Vector2D_RndRead_STL)->Arg(1000)->Arg(10000)->Arg(100000)->Arg(1000000);


void Vector2D_RndRead_Yato(benchmark::State& state)
{
    static const int RND_SEED = 3;
    std::srand(RND_SEED);

    const size_t N = yato::narrow_cast<size_t>(state.range(0));

    const size_t S0 = yato::narrow_cast<size_t>(10 + std::rand() % 2);
    const size_t S1 = yato::narrow_cast<size_t>(10 + std::rand() % 2);
    yato::vector_2d<int32_t> vec(yato::dims(S0, S1));
    std::generate(vec.plain_begin(), vec.plain_end(), std::rand);

    std::vector<std::tuple<size_t, size_t>> indexes(N);
    std::generate(indexes.begin(), indexes.end(), [S0, S1]() { return std::make_tuple(
        yato::narrow_cast<size_t>(std::rand() % S0),
        yato::narrow_cast<size_t>(std::rand() % S1));
        });

    for (auto _ : state) {
        uint64_t sum = 0;
        for (size_t n = 0; n < N; ++n) {
            size_t i0, i1;
            std::tie(i0, i1) = indexes[n];
            sum += vec[i0][i1];
        }
        benchmark::DoNotOptimize(sum);
    }
}
BENCHMARK(Vector2D_RndRead_Yato)->Arg(1000)->Arg(10000)->Arg(100000)->Arg(1000000);



void Vector3D_RndRead_STL(benchmark::State& state)
{
    static const int RND_SEED = 3;
    std::srand(RND_SEED);

    const size_t N = yato::narrow_cast<size_t>(state.range(0));

    const size_t S0 = yato::narrow_cast<size_t>(5 + std::rand() % 2);
    const size_t S1 = yato::narrow_cast<size_t>(5 + std::rand() % 2);
    const size_t S2 = yato::narrow_cast<size_t>(5 + std::rand() % 2);
    std::vector<int32_t> vec(S0 * S1 * S2);
    std::generate(vec.begin(), vec.end(), std::rand);

    std::vector<std::tuple<size_t, size_t, size_t>> indexes(N);
    std::generate(indexes.begin(), indexes.end(), [S0, S1, S2]() { return std::make_tuple(
        yato::narrow_cast<size_t>(std::rand() % S0),
        yato::narrow_cast<size_t>(std::rand() % S1),
        yato::narrow_cast<size_t>(std::rand() % S2));
        });

    for (auto _ : state) {
        uint64_t sum = 0;
        for (size_t n = 0; n < N; ++n) {
            size_t i0, i1, i2;
            std::tie(i0, i1, i2) = indexes[n];
            sum += vec[(i0 * S1 + i1) * S2 + i2];
        }
        benchmark::DoNotOptimize(sum);
    }
}
BENCHMARK(Vector3D_RndRead_STL)->Arg(1000)->Arg(10000)->Arg(100000)->Arg(1000000);


void Vector3D_RndRead_Yato(benchmark::State& state)
{
    static const int RND_SEED = 3;
    std::srand(RND_SEED);

    const size_t N = yato::narrow_cast<size_t>(state.range(0));

    const size_t S0 = yato::narrow_cast<size_t>(5 + std::rand() % 2);
    const size_t S1 = yato::narrow_cast<size_t>(5 + std::rand() % 2);
    const size_t S2 = yato::narrow_cast<size_t>(5 + std::rand() % 2);
    yato::vector_3d<int32_t> vec(yato::dims(S0, S1, S2));
    std::generate(vec.plain_begin(), vec.plain_end(), std::rand);

    std::vector<std::tuple<size_t, size_t, size_t>> indexes(N);
    std::generate(indexes.begin(), indexes.end(), [S0, S1, S2]() { return std::make_tuple(
        yato::narrow_cast<size_t>(std::rand() % S0),
        yato::narrow_cast<size_t>(std::rand() % S1),
        yato::narrow_cast<size_t>(std::rand() % S2));
        });

    for (auto _ : state) {
        uint64_t sum = 0;
        for (size_t n = 0; n < N; ++n) {
            size_t i0, i1, i2;
            std::tie(i0, i1, i2) = indexes[n];
            sum += vec[i0][i1][i2];
        }
        benchmark::DoNotOptimize(sum);
    }
}
BENCHMARK(Vector3D_RndRead_Yato)->Arg(1000)->Arg(10000)->Arg(100000)->Arg(1000000);




void Vector1D_PushBack_STL(benchmark::State& state)
{
    static const int RND_SEED = 3;
    const size_t N = state.range(0);

    std::vector<int32_t> test;
    std::srand(RND_SEED);

    for (auto _ : state) {
        for (size_t i = 0; i < N; ++i) {
            test.push_back(std::rand());
        }
    }

    const int32_t sum = std::accumulate(test.cbegin(), test.cend(), 0);
    benchmark::DoNotOptimize(sum);
}
BENCHMARK(Vector1D_PushBack_STL)->Arg(1000)->Arg(10000)->Arg(100000)->Arg(1000000);



void Vector1D_PushBack_Yato(benchmark::State& state)
{
    static const int RND_SEED = 3;
    const size_t N = state.range(0);

    yato::vector_1d<int32_t> test;
    std::srand(RND_SEED);

    for (auto _ : state) {
        for (size_t i = 0; i < N; ++i) {
            test.push_back(std::rand());
        }
    }

    const int32_t sum = std::accumulate(test.cbegin(), test.cend(), 0);
    benchmark::DoNotOptimize(sum);
}
BENCHMARK(Vector1D_PushBack_Yato)->Arg(1000)->Arg(10000)->Arg(100000)->Arg(1000000);

