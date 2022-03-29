#include "thh_handles/thh_handles.hpp"

#include <benchmark/benchmark.h>

static void add_element(benchmark::State& state)
{
  thh::handle_vector_t<int> handle_vector;
  for ([[maybe_unused]] auto _ : state) {
    const thh::handle_t handle = handle_vector.add();
    benchmark::DoNotOptimize(handle);
    benchmark::ClobberMemory();
  }
}

BENCHMARK(add_element);

static void add_element_with_reserve(benchmark::State& state)
{
  thh::handle_vector_t<int> handle_vector;
  handle_vector.reserve(8);
  for ([[maybe_unused]] auto _ : state) {
    const thh::handle_t handle = handle_vector.add();
    benchmark::DoNotOptimize(handle);
    benchmark::ClobberMemory();
  }
}

BENCHMARK(add_element_with_reserve);

static void remove_element(benchmark::State& state)
{
  thh::handle_vector_t<int> handle_vector;
  const thh::handle_t handle = handle_vector.add();
  for ([[maybe_unused]] auto _ : state) {
    handle_vector.remove(handle);
    benchmark::ClobberMemory();
  }
}

BENCHMARK(remove_element);

static void has_element_present(benchmark::State& state)
{
  thh::handle_vector_t<int> handle_vector;
  thh::handle_t handle = handle_vector.add();
  for ([[maybe_unused]] auto _ : state) {
    benchmark::DoNotOptimize(handle_vector.has(handle));
  }
}

BENCHMARK(has_element_present);

static void has_element_not_present(benchmark::State& state)
{
  thh::handle_vector_t<int> handle_vector;
  thh::handle_t handle = handle_vector.add();
  handle_vector.remove(handle);
  for ([[maybe_unused]] auto _ : state) {
    benchmark::DoNotOptimize(handle_vector.has(handle));
  }
}

BENCHMARK(has_element_not_present);

static void resolve(benchmark::State& state)
{
  thh::handle_vector_t<int> handle_vector;
  thh::handle_t handle = handle_vector.add();
  for ([[maybe_unused]] auto _ : state) {
    handle_vector.call(handle, [](const auto& element) {
      benchmark::DoNotOptimize(element);
      benchmark::ClobberMemory();
    });
  }
}

BENCHMARK(resolve);

static void enumerate_range_for(benchmark::State& state)
{
  thh::handle_vector_t<int> handle_vector;
  std::vector<thh::handle_t> handles;
  handles.reserve(10);
  for (int i = 0; i < 10; ++i) {
    handles.push_back(handle_vector.add());
  }
  for ([[maybe_unused]] auto _ : state) {
    for (auto& element : handle_vector) {
      int i = 0;
      element = i++;
      benchmark::ClobberMemory();
    }
  }
}

BENCHMARK(enumerate_range_for);

static void enumerate_resolve(benchmark::State& state)
{
  thh::handle_vector_t<int> handle_vector;
  std::vector<thh::handle_t> handles;
  handles.reserve(10);
  for (int i = 0; i < 10; ++i) {
    handles.push_back(handle_vector.add());
  }
  for ([[maybe_unused]] auto _ : state) {
    for (int64_t i = 0; i < handle_vector.size(); ++i) {
      handle_vector.call(handles[i], [i](auto& element) {
        element = static_cast<int>(i);
        benchmark::ClobberMemory();
      });
    }
  }
}

BENCHMARK(enumerate_resolve);

static void enumerate_iterators(benchmark::State& state)
{
  thh::handle_vector_t<int> handle_vector;
  std::vector<thh::handle_t> handles;
  handles.reserve(10);
  for (int i = 0; i < 10; ++i) {
    handles.push_back(handle_vector.add());
  }
  for ([[maybe_unused]] auto _ : state) {
    int i = 0;
    for (auto& element : handle_vector) {
      element = i++;
      benchmark::ClobberMemory();
    }
  }
}

BENCHMARK(enumerate_iterators);

BENCHMARK_MAIN();
