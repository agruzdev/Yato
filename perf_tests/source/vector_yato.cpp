/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2018 Alexey Gruzdev
*/

#include <algorithm>
#include <cmath>

#include <benchmark/benchmark.h>

#include <yato/vector_nd.h>


void Vector1D_Write_Yato(benchmark::State & state)
{
    static const int RND_SEED = 3;
    const size_t N = state.range(0);

    std::vector<int32_t> src(N);
    yato::vector_1d<int32_t> dst(N);

    std::srand(RND_SEED);
    std::generate(src.begin(), src.end(), std::rand);

    for (auto _ : state) {
        const int32_t* input = src.data();
        for(size_t i = 0; i < N; ++i) {
            benchmark::DoNotOptimize(dst[i] = *input++);
        }
    }
}


BENCHMARK(Vector1D_Write_Yato)->Arg(1000)->Arg(10000)->Arg(100000)->Arg(1000000);


void Vector2D_Write_Yato(benchmark::State & state)
{
    static const int RND_SEED = 3;
    const size_t TOTAL_SIZE = state.range(0);
    const size_t N = static_cast<size_t>(std::sqrt(TOTAL_SIZE)); 

    std::vector<int32_t> src(N * N);
    yato::vector_2d<int32_t> dst(yato::dims(N, N));

    std::srand(RND_SEED);
    std::generate(src.begin(), src.end(), std::rand);

    for (auto _ : state) {
        const int32_t* input = src.data();
        for(size_t y = 0; y < N; ++y) {
            for(size_t x = 0; x < N; ++x) {
                benchmark::DoNotOptimize(dst[y][x] = *input++);
            }
        }
    }
}

BENCHMARK(Vector2D_Write_Yato)->Arg(1000)->Arg(10000)->Arg(100000)->Arg(1000000);


void Vector3D_Write_Yato(benchmark::State & state)
{
    static const int RND_SEED = 3;
    const size_t TOTAL_SIZE = state.range(0);
    const size_t N = static_cast<size_t>(std::cbrt(TOTAL_SIZE)); 

    std::vector<int32_t> src(N * N * N);
    yato::vector_3d<int32_t> dst(yato::dims(N, N, N));

    std::srand(RND_SEED);
    std::generate(src.begin(), src.end(), std::rand);

    for (auto _ : state) {
        const int32_t* input = src.data();
        for(size_t z = 0; z < N; ++z) {
            for(size_t y = 0; y < N; ++y) {
                for(size_t x = 0; x < N; ++x) {
                    benchmark::DoNotOptimize(dst[z][y][x] = *input++);
                }
            }
        }
    }
}

BENCHMARK(Vector3D_Write_Yato)->Arg(1000)->Arg(10000)->Arg(100000)->Arg(1000000);
