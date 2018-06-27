/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2018 Alexey Gruzdev
*/

#include <algorithm>
#include <cmath>

#include <benchmark/benchmark.h>


void Vector1D_Write_STL(benchmark::State & state)
{
    static const int RND_SEED = 3;
    const size_t N = state.range(0);

    std::vector<int32_t> src(N);
    std::vector<int32_t> dst(N);

    std::srand(RND_SEED);
    std::generate(src.begin(), src.end(), std::rand);

    for (auto _ : state) {
        const int32_t* input = src.data();
        for(size_t i = 0; i < N; ++i) {
            dst[i] = *input++;
        }
    }

    benchmark::DoNotOptimize(dst[std::rand() % dst.size()]);
}


BENCHMARK(Vector1D_Write_STL)->Arg(1000)->Arg(10000)->Arg(100000)->Arg(1000000);


void Vector2D_Write_STL(benchmark::State & state)
{
    static const int RND_SEED = 3;
    const size_t TOTAL_SIZE = state.range(0);
    const size_t N = static_cast<size_t>(std::sqrt(TOTAL_SIZE)); 

    std::vector<int32_t> src(N * N);
    std::vector<int32_t> dst(N * N);

    std::srand(RND_SEED);
    std::generate(src.begin(), src.end(), std::rand);

    for (auto _ : state) {
        const int32_t* input = src.data();
        for(size_t y = 0; y < N; ++y) {
            for(size_t x = 0; x < N; ++x) {
                dst[y * N + x] = *input++;
            }
        }
    }

    benchmark::DoNotOptimize(dst[std::rand() % dst.size()]);
}

BENCHMARK(Vector2D_Write_STL)->Arg(1000)->Arg(10000)->Arg(100000)->Arg(1000000);


void Vector3D_Write_STL(benchmark::State & state)
{
    static const int RND_SEED = 3;
    const size_t TOTAL_SIZE = state.range(0);
    const size_t N = static_cast<size_t>(std::cbrt(TOTAL_SIZE)); 

    std::vector<int32_t> src(N * N * N);
    std::vector<int32_t> dst(N * N * N);

    std::srand(RND_SEED);
    std::generate(src.begin(), src.end(), std::rand);

    for (auto _ : state) {
        const int32_t* input = src.data();
        for(size_t z = 0; z < N; ++z) {
            for(size_t y = 0; y < N; ++y) {
                for(size_t x = 0; x < N; ++x) {
                    dst[(z * N + y) * N + x] = *input++;
                }
            }
        }
    }

    benchmark::DoNotOptimize(dst[std::rand() % dst.size()]);
}

BENCHMARK(Vector3D_Write_STL)->Arg(1000)->Arg(10000)->Arg(100000)->Arg(1000000);
