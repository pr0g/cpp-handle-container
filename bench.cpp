#include "thh_handles/thh_handles.hpp"

#include <benchmark/benchmark.h>

static void add_element(benchmark::State& state)
{
  thh::handle_vector_t<int> container;
  for ([[maybe_unused]] auto _ : state) {
    const thh::handle_t handle = container.add();
    benchmark::DoNotOptimize(handle);
    benchmark::ClobberMemory();
  }
}

BENCHMARK(add_element);

static void add_element_with_reserve(benchmark::State& state)
{
  thh::handle_vector_t<int> container;
  container.reserve(8);
  for ([[maybe_unused]] auto _ : state) {
    const thh::handle_t handle = container.add();
    benchmark::DoNotOptimize(handle);
    benchmark::ClobberMemory();
  }
}

BENCHMARK(add_element_with_reserve);

static void remove_element(benchmark::State& state)
{
  thh::handle_vector_t<int> container;
  const thh::handle_t handle = container.add();
  for ([[maybe_unused]] auto _ : state) {
    container.remove(handle);
    benchmark::ClobberMemory();
  }
}

BENCHMARK(remove_element);

static void has_element_present(benchmark::State& state)
{
  thh::handle_vector_t<int> container;
  thh::handle_t handle = container.add();
  for ([[maybe_unused]] auto _ : state) {
    benchmark::DoNotOptimize(container.has(handle));
  }
}

BENCHMARK(has_element_present);

static void has_element_not_present(benchmark::State& state)
{
  thh::handle_vector_t<int> container;
  thh::handle_t handle = container.add();
  container.remove(handle);
  for ([[maybe_unused]] auto _ : state) {
    benchmark::DoNotOptimize(container.has(handle));
  }
}

BENCHMARK(has_element_not_present);

static void resolve(benchmark::State& state)
{
  thh::handle_vector_t<int> container;
  thh::handle_t handle = container.add();
  for ([[maybe_unused]] auto _ : state) {
    container.call(handle, [](const auto& element) {
      benchmark::DoNotOptimize(element);
      benchmark::ClobberMemory();
    });
  }
}

BENCHMARK(resolve);

static void enumerate_callback(benchmark::State& state)
{
  thh::handle_vector_t<int> container;
  std::vector<thh::handle_t> handles;
  handles.reserve(10);
  for (int i = 0; i < 10; ++i) {
    handles.push_back(container.add());
  }
  for ([[maybe_unused]] auto _ : state) {
    container.enumerate([i = 0](auto& element) mutable {
      element = i++;
      benchmark::ClobberMemory();
    });
  }
}

BENCHMARK(enumerate_callback);

static void enumerate_callback_resolve(benchmark::State& state)
{
  thh::handle_vector_t<int> container;
  std::vector<thh::handle_t> handles;
  handles.reserve(10);
  for (int i = 0; i < 10; ++i) {
    handles.push_back(container.add());
  }
  for ([[maybe_unused]] auto _ : state) {
    for (int64_t i = 0; i < container.size(); ++i) {
      container.call(handles[i], [i](auto& element) {
        element = static_cast<int>(i);
        benchmark::ClobberMemory();
      });
    }
  }
}

BENCHMARK(enumerate_callback_resolve);

static void enumerate_iterators(benchmark::State& state)
{
  thh::handle_vector_t<int> container;
  std::vector<thh::handle_t> handles;
  handles.reserve(10);
  for (int i = 0; i < 10; ++i) {
    handles.push_back(container.add());
  }
  for ([[maybe_unused]] auto _ : state) {
    int i = 0;
    for (auto& element : container) {
      element = i++;
      benchmark::ClobberMemory();
    }
  }
}

BENCHMARK(enumerate_iterators);

BENCHMARK_MAIN();
