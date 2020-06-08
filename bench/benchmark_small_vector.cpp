
#include <array>
#include <random>
#include <vector>
#include <algorithm>
#include <chrono>

#include "small_vector\small_vector.h"

#include <benchmark/benchmark.h>

template<typename T>
using s_vector_8 = sbo::small_vector<T, 8>;
template<typename T>
using s_vector_16 = sbo::small_vector<T, 16>;

template<int N>
class NonTrivialArray {
public:
    std::size_t a = 0;

private:
    std::array<unsigned char, N - sizeof(a)> b;

public:
    NonTrivialArray() = default;
    NonTrivialArray(std::size_t a) : a(a) {}
    ~NonTrivialArray() = default;
    bool operator<(const NonTrivialArray& other) const { return a < other.a; }
};

template<typename ContainerT>
static void ConstructWithSize(benchmark::State& state) {
    
    for (auto _ : state) {
        (void)_;
        ContainerT v(state.range(0));
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
        ContainerT v;
        v.reserve(state.range(0));
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
        state.ResumeTiming();
        benchmark::DoNotOptimize(v.data());
        for (std::size_t i = 0; i < state.range(0); ++i) {
            auto val = distribution(generator);
            // hand written comparison to eliminate temporary object creation
            v.insert(std::lower_bound(begin(v), end(v), val), val);
        }
        benchmark::ClobberMemory();
    }
}


BENCHMARK_TEMPLATE(DefaultConstruct, std::vector<int>)->RangeMultiplier(2)->Range(8, 256);
BENCHMARK_TEMPLATE(DefaultConstruct, s_vector_8<int>)->RangeMultiplier(2)->Range(8, 256);
BENCHMARK_TEMPLATE(DefaultConstruct, s_vector_16<int>)->RangeMultiplier(2)->Range(8, 256);
// Register the function as a benchmark
BENCHMARK_TEMPLATE(ConstructWithSize, std::vector<int>)->RangeMultiplier(2)->Range(8, 256);
BENCHMARK_TEMPLATE(ConstructWithSize, s_vector_8<int>)->RangeMultiplier(2)->Range(8, 256);
BENCHMARK_TEMPLATE(ConstructWithSize, s_vector_16<int>)->RangeMultiplier(2)->Range(8, 256);

BENCHMARK_TEMPLATE(ConstructWithSize, std::vector<NonTrivialArray<32>>)->RangeMultiplier(2)->Range(8, 256);
BENCHMARK_TEMPLATE(ConstructWithSize, s_vector_8<NonTrivialArray<32>>)->RangeMultiplier(2)->Range(8, 256);
BENCHMARK_TEMPLATE(ConstructWithSize, s_vector_16<NonTrivialArray<32>>)->RangeMultiplier(2)->Range(8, 256);

BENCHMARK_TEMPLATE(EmplaceBack, std::vector<int>)->RangeMultiplier(2)->Range(8, 256);
BENCHMARK_TEMPLATE(EmplaceBack, s_vector_8<int>)->RangeMultiplier(2)->Range(8, 256);
BENCHMARK_TEMPLATE(EmplaceBack, s_vector_16<int>)->RangeMultiplier(2)->Range(8, 256);

BENCHMARK_TEMPLATE(EmplaceBackReserve, std::vector<int>)->RangeMultiplier(2)->Range(8, 256);
BENCHMARK_TEMPLATE(EmplaceBackReserve, s_vector_8<int>)->RangeMultiplier(2)->Range(8, 256);
BENCHMARK_TEMPLATE(EmplaceBackReserve, s_vector_16<int>)->RangeMultiplier(2)->Range(8, 256);

BENCHMARK_TEMPLATE(EmplaceBackReserve, std::vector<NonTrivialArray<32>>)->RangeMultiplier(2)->Range(8, 256);
BENCHMARK_TEMPLATE(EmplaceBackReserve, s_vector_8<NonTrivialArray<32>>)->RangeMultiplier(2)->Range(8, 256);
BENCHMARK_TEMPLATE(EmplaceBackReserve, s_vector_16<NonTrivialArray<32>>)->RangeMultiplier(2)->Range(8, 256);

BENCHMARK_TEMPLATE(RandomSortedInsertion, std::vector<size_t>)->RangeMultiplier(2)->Range(8, 256);
BENCHMARK_TEMPLATE(RandomSortedInsertion, s_vector_8<size_t>)->RangeMultiplier(2)->Range(8, 256);
BENCHMARK_TEMPLATE(RandomSortedInsertion, s_vector_16<size_t>)->RangeMultiplier(2)->Range(8, 256);
// Run the benchmark
BENCHMARK_MAIN();