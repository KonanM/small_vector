// Licensed under the Unlicense <https://unlicense.org/>
// SPDX-License-Identifier: Unlicense
#include <vector>
#include <algorithm>
#include <random>

#include "SmallVector.h"
#include "small_vector\small_vector.h"

#include <benchmark/benchmark.h>

template<typename ContainerT>
static void ConstructWithSize(benchmark::State& state) {
    
    for (auto _ : state) {
        (void)_;
        ContainerT v(static_cast<size_t>(state.range(0)));
        benchmark::DoNotOptimize(v.data());
        benchmark::ClobberMemory();
    }
}

template<typename ContainerT>
static void DefaultConstruct(benchmark::State& state) {

    for (auto _ : state) {
        (void)_;
        ContainerT v;
        benchmark::DoNotOptimize(v.data());
        benchmark::ClobberMemory();
    }
}

template<typename ContainerT>
static void EmplaceBack(benchmark::State& state) {

    for (auto _ : state) {
        (void)_;
        state.PauseTiming();
        ContainerT v;
        state.ResumeTiming();
        for (int j = 0; j < state.range(0); ++j)
            v.emplace_back();
        benchmark::DoNotOptimize(v.data());
        benchmark::ClobberMemory();
    }
}

template<typename ContainerT>
static void EmplaceBackReserve(benchmark::State& state) {

    for (auto _ : state) {
        (void)_;
        state.PauseTiming();
        ContainerT v;
        state.ResumeTiming();
        v.reserve(static_cast<size_t>(state.range(0)));
        for (int j = 0; j < state.range(0); ++j)
            v.emplace_back();
        benchmark::DoNotOptimize(v.data());
        benchmark::ClobberMemory();
    }
}

template<typename ContainerT>
static void RandomSortedInsertion(benchmark::State& state) {
    static std::mt19937 generator;
    static std::uniform_int_distribution<std::size_t> distribution;
    for (auto _ : state) {
        (void)_;
        state.PauseTiming();
        ContainerT v;
        v.resize(static_cast<size_t>(state.range(0)));
        state.ResumeTiming();
        benchmark::DoNotOptimize(v.data());
        for (std::size_t i = 0; i < static_cast<size_t>(state.range(0)); ++i) {
            auto val = distribution(generator);
            v.insert(std::lower_bound(v.begin(), v.end(), val), val);
        }
        benchmark::ClobberMemory();
    }
}


BENCHMARK_TEMPLATE(DefaultConstruct, std::vector<int>)->RangeMultiplier(2)->Range(8, 256);
BENCHMARK_TEMPLATE(DefaultConstruct, sbo::small_vector<int, 8>)->RangeMultiplier(2)->Range(8, 256);
BENCHMARK_TEMPLATE(DefaultConstruct, llvm_vecsmall::SmallVector<int, 8>)->RangeMultiplier(2)->Range(8, 256);
BENCHMARK_TEMPLATE(DefaultConstruct, sbo::small_vector<int, 16>)->RangeMultiplier(2)->Range(8, 256);
// Register the function as a benchmark
BENCHMARK_TEMPLATE(ConstructWithSize, std::vector<int>)->RangeMultiplier(2)->Range(8, 256);
BENCHMARK_TEMPLATE(ConstructWithSize, sbo::small_vector<int, 8>)->RangeMultiplier(2)->Range(8, 256);
BENCHMARK_TEMPLATE(ConstructWithSize, llvm_vecsmall::SmallVector<int, 8>)->RangeMultiplier(2)->Range(8, 256);
BENCHMARK_TEMPLATE(ConstructWithSize, sbo::small_vector<int, 16>)->RangeMultiplier(2)->Range(8, 256);

BENCHMARK_TEMPLATE(ConstructWithSize, std::vector<std::string>)->RangeMultiplier(2)->Range(8, 256);
BENCHMARK_TEMPLATE(ConstructWithSize, llvm_vecsmall::SmallVector<int, 8>)->RangeMultiplier(2)->Range(8, 256);
BENCHMARK_TEMPLATE(ConstructWithSize, sbo::small_vector<std::string, 8>)->RangeMultiplier(2)->Range(8, 256);
BENCHMARK_TEMPLATE(ConstructWithSize, sbo::small_vector<std::string, 16>)->RangeMultiplier(2)->Range(8, 256);

BENCHMARK_TEMPLATE(EmplaceBack, std::vector<int>)->RangeMultiplier(2)->Range(8, 256);
BENCHMARK_TEMPLATE(EmplaceBack, sbo::small_vector<int, 8>)->RangeMultiplier(2)->Range(8, 256);
BENCHMARK_TEMPLATE(EmplaceBack, llvm_vecsmall::SmallVector<int, 8>)->RangeMultiplier(2)->Range(8, 256);
BENCHMARK_TEMPLATE(EmplaceBack, sbo::small_vector<int, 16>)->RangeMultiplier(2)->Range(8, 256);

BENCHMARK_TEMPLATE(EmplaceBackReserve, std::vector<int>)->RangeMultiplier(2)->Range(8, 256);
BENCHMARK_TEMPLATE(EmplaceBackReserve, sbo::small_vector<int, 8>)->RangeMultiplier(2)->Range(8, 256);
BENCHMARK_TEMPLATE(EmplaceBackReserve, llvm_vecsmall::SmallVector<int, 8>)->RangeMultiplier(2)->Range(8, 256);
BENCHMARK_TEMPLATE(EmplaceBackReserve, sbo::small_vector<int, 16>)->RangeMultiplier(2)->Range(8, 256);

BENCHMARK_TEMPLATE(EmplaceBackReserve, std::vector<std::string>)->RangeMultiplier(2)->Range(8, 256);
BENCHMARK_TEMPLATE(EmplaceBackReserve, sbo::small_vector<std::string, 8>)->RangeMultiplier(2)->Range(8, 256);
BENCHMARK_TEMPLATE(EmplaceBackReserve, llvm_vecsmall::SmallVector<std::string, 8>)->RangeMultiplier(2)->Range(8, 256);
BENCHMARK_TEMPLATE(EmplaceBackReserve, sbo::small_vector<std::string, 16>)->RangeMultiplier(2)->Range(8, 256);

BENCHMARK_TEMPLATE(RandomSortedInsertion, std::vector<size_t>)->RangeMultiplier(2)->Range(8, 256);
BENCHMARK_TEMPLATE(RandomSortedInsertion, sbo::small_vector<size_t, 8>)->RangeMultiplier(2)->Range(8, 256);
BENCHMARK_TEMPLATE(RandomSortedInsertion, llvm_vecsmall::SmallVector<size_t, 8>)->RangeMultiplier(2)->Range(8, 256);
BENCHMARK_TEMPLATE(RandomSortedInsertion, sbo::small_vector<size_t, 16>)->RangeMultiplier(2)->Range(8, 256);
// Run the benchmark
BENCHMARK_MAIN();
