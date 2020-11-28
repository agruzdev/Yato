/**
* YATO library
*
* Apache License, Version 2.0
* Copyright (c) 2016-2020 Alexey Gruzdev
*/

#include <algorithm>
#include <cmath>
#include <numeric>

#include <benchmark/benchmark.h>

#include <yato/vector_nd.h>


void Fetch1D_STL_At(benchmark::State & state)
{
    static const int RND_SEED = 3;
    std::srand(RND_SEED);

    const int32_t N = yato::narrow_cast<int32_t>(state.range(0));

    const int32_t S0 = 10 + rand() % 2;
    std::vector<int32_t> vec(S0);
    std::generate(vec.begin(), vec.end(), std::rand);

    const int32_t i = rand() % (S0 - 3);
    benchmark::DoNotOptimize(i);

    int32_t sum = rand();
    for (auto _ : state) {
        for (int32_t n = 0; n < N; ++n) {
            sum &= vec.at(i + (n & 0x3));
        }
        benchmark::DoNotOptimize(sum);
    }
}

BENCHMARK(Fetch1D_STL_At)->Arg(1)->Arg(1000)->Arg(10000)->Arg(100000)->Arg(1000000);


template <typename Sampler_>
void Fetch1D_Yato(benchmark::State& state)
{
    static const int RND_SEED = 3;
    std::srand(RND_SEED);

    const int32_t N = yato::narrow_cast<int32_t>(state.range(0));

    const int32_t S0 = 10 + rand() % 2;
    yato::vector_1d<int32_t> vec(S0);
    std::generate(vec.begin(), vec.end(), std::rand);

    const int32_t i = rand() % (S0 - 3);
    benchmark::DoNotOptimize(i);

    int32_t sum = rand();
    for (auto _ : state) {
        for (int32_t n = 0; n < N; ++n) {
            sum &= vec.at<Sampler_>(i + (n & 0x3));
        }
        benchmark::DoNotOptimize(sum);
    }
}

BENCHMARK_TEMPLATE(Fetch1D_Yato, yato::sampler_default)->Arg(1)->Arg(1000)->Arg(10000)->Arg(100000)->Arg(1000000);
BENCHMARK_TEMPLATE(Fetch1D_Yato, yato::sampler_no_check)->Arg(1)->Arg(1000)->Arg(10000)->Arg(100000)->Arg(1000000);
BENCHMARK_TEMPLATE(Fetch1D_Yato, yato::sampler_clamp)->Arg(1)->Arg(1000)->Arg(10000)->Arg(100000)->Arg(1000000);
BENCHMARK_TEMPLATE(Fetch1D_Yato, yato::sampler_zero)->Arg(1)->Arg(1000)->Arg(10000)->Arg(100000)->Arg(1000000);


void Fetch2D_STL_At(benchmark::State& state)
{
    static const int RND_SEED = 3;
    std::srand(RND_SEED);

    const int32_t N = yato::narrow_cast<int32_t>(state.range(0));

    const int32_t S0 = 10 + rand() % 2;
    const int32_t S1 = 10 + rand() % 2;
    std::vector<int32_t> vec(S0 * S1);
    std::generate(vec.begin(), vec.end(), std::rand);

    const int32_t i0 = rand() % (S0 - 3);
    const int32_t i1 = rand() % (S1 - 3);
    benchmark::DoNotOptimize(i0);
    benchmark::DoNotOptimize(i1);

    auto at2 = [S0, S1](const std::vector<int32_t>& v, int32_t i0, int32_t i1) -> const int32_t& {
        if (i0 >= S0 || i1 >= S1) {
            throw std::out_of_range("at() out_of_range");
        }
        return v[i0 * S1 + i1];
    };

    int32_t sum = rand();
    for (auto _ : state) {
        for (int32_t n = 0; n < N; ++n) {
            sum &= at2(vec, i0 + (n & 0x1), i1 + (n & 0x3));
        }
        benchmark::DoNotOptimize(sum);
    }
}

BENCHMARK(Fetch2D_STL_At)->Arg(1)->Arg(1000)->Arg(10000)->Arg(100000)->Arg(1000000);

template <typename Sampler_>
void Fetch2D_Yato(benchmark::State& state)
{
    static const int RND_SEED = 3;
    std::srand(RND_SEED);

    const int32_t N = yato::narrow_cast<int32_t>(state.range(0));

    const int32_t S0 = 10 + rand() % 2;
    const int32_t S1 = 10 + rand() % 2;
    yato::vector_2d<int32_t> vec(yato::dims(S0, S1));
    std::generate(vec.plain_begin(), vec.plain_end(), std::rand);

    const int32_t i0 = rand() % (S0 - 3);
    const int32_t i1 = rand() % (S1 - 3);
    benchmark::DoNotOptimize(i0);
    benchmark::DoNotOptimize(i1);

    int32_t sum = rand();
    for (auto _ : state) {
        for (int32_t n = 0; n < N; ++n) {
            sum &= vec.at<Sampler_>(i0 + (n & 0x1), i1 + (n & 0x3));
        }
        benchmark::DoNotOptimize(sum);
    }
}

BENCHMARK_TEMPLATE(Fetch2D_Yato, yato::sampler_default)->Arg(1)->Arg(1000)->Arg(10000)->Arg(100000)->Arg(1000000);
BENCHMARK_TEMPLATE(Fetch2D_Yato, yato::sampler_no_check)->Arg(1)->Arg(1000)->Arg(10000)->Arg(100000)->Arg(1000000);
BENCHMARK_TEMPLATE(Fetch2D_Yato, yato::sampler_clamp)->Arg(1)->Arg(1000)->Arg(10000)->Arg(100000)->Arg(1000000);
BENCHMARK_TEMPLATE(Fetch2D_Yato, yato::sampler_zero)->Arg(1)->Arg(1000)->Arg(10000)->Arg(100000)->Arg(1000000);



void Fetch3D_STL_At(benchmark::State& state)
{
    static const int RND_SEED = 3;
    std::srand(RND_SEED);

    const int32_t N = yato::narrow_cast<int32_t>(state.range(0));

    const int32_t S0 = 10 + rand() % 2;
    const int32_t S1 = 10 + rand() % 2;
    const int32_t S2 = 10 + rand() % 2;
    std::vector<int32_t> vec(S0 * S1 * S2);
    std::generate(vec.begin(), vec.end(), std::rand);

    const int32_t i0 = rand() % (S0 - 3);
    const int32_t i1 = rand() % (S1 - 3);
    const int32_t i2 = rand() % (S2 - 3);
    benchmark::DoNotOptimize(i0);
    benchmark::DoNotOptimize(i1);
    benchmark::DoNotOptimize(i2);

    auto at3 = [S0, S1, S2](const std::vector<int32_t>& v, int32_t i0, int32_t i1, int32_t i2) -> const int32_t& {
        if (i0 >= S0 || i1 >= S1 || i2 >= S2) {
            throw std::out_of_range("at() out_of_range");
        }
        return v[(i0 * S1 + i1) * S2 + i2];
    };

    int32_t sum = rand();
    for (auto _ : state) {
        for (int32_t n = 0; n < N; ++n) {
            sum &= at3(vec, i0 + (n & 0x1), i1 + (n & 0x1), i2 + (n & 0x3));
        }
        benchmark::DoNotOptimize(sum);
    }
}

BENCHMARK(Fetch3D_STL_At)->Arg(1)->Arg(1000)->Arg(10000)->Arg(100000)->Arg(1000000);

template <typename Sampler_>
void Fetch3D_Yato(benchmark::State& state)
{
    static const int RND_SEED = 3;
    std::srand(RND_SEED);

    const int32_t N = yato::narrow_cast<int32_t>(state.range(0));

    const int32_t S0 = 10 + rand() % 2;
    const int32_t S1 = 10 + rand() % 2;
    const int32_t S2 = 10 + rand() % 2;
    yato::vector_3d<int32_t> vec(yato::dims(S0, S1, S2));
    std::generate(vec.plain_begin(), vec.plain_end(), std::rand);

    const int32_t i0 = rand() % (S0 - 3);
    const int32_t i1 = rand() % (S1 - 3);
    const int32_t i2 = rand() % (S2 - 3);
    benchmark::DoNotOptimize(i0);
    benchmark::DoNotOptimize(i1);
    benchmark::DoNotOptimize(i2);

    int32_t sum = rand();
    for (auto _ : state) {
        for (int32_t n = 0; n < N; ++n) {
            sum &= vec.at<Sampler_>(i0 + (n & 0x1), i1 + (n & 0x1), i2 + (n & 0x3));
        }
        benchmark::DoNotOptimize(sum);
    }
}

BENCHMARK_TEMPLATE(Fetch3D_Yato, yato::sampler_default)->Arg(1)->Arg(1000)->Arg(10000)->Arg(100000)->Arg(1000000);
BENCHMARK_TEMPLATE(Fetch3D_Yato, yato::sampler_no_check)->Arg(1)->Arg(1000)->Arg(10000)->Arg(100000)->Arg(1000000);
BENCHMARK_TEMPLATE(Fetch3D_Yato, yato::sampler_clamp)->Arg(1)->Arg(1000)->Arg(10000)->Arg(100000)->Arg(1000000);
BENCHMARK_TEMPLATE(Fetch3D_Yato, yato::sampler_zero)->Arg(1)->Arg(1000)->Arg(10000)->Arg(100000)->Arg(1000000);
