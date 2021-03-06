#include "thh_handles/thh_handles.hpp"

#include <benchmark/benchmark.h>

static void AddElement(benchmark::State& state) {
  thh::container_t<int> container;
  for (auto _ : state) {
    const thh::handle_t handle = container.add();
    benchmark::DoNotOptimize(handle);
    benchmark::ClobberMemory();
  }
}

BENCHMARK(AddElement);

static void AddElementWithReserve(benchmark::State& state) {
  thh::container_t<int> container;
  container.reserve(8);
  for (auto _ : state) {
    const thh::handle_t handle = container.add();
    benchmark::DoNotOptimize(handle);
    benchmark::ClobberMemory();
  }
}

BENCHMARK(AddElementWithReserve);

static void RemoveElement(benchmark::State& state) {
  thh::container_t<int> container;
  const thh::handle_t handle = container.add();
  for (auto _ : state) {
    container.remove(handle);
    benchmark::ClobberMemory();
  }
}

BENCHMARK(RemoveElement);

static void HasElementPresent(benchmark::State& state) {
  thh::container_t<int> container;
  thh::handle_t handle = container.add();
  for (auto _ : state) {
    benchmark::DoNotOptimize(container.has(handle));
  }
}

BENCHMARK(HasElementPresent);

static void HasElementNotPresent(benchmark::State& state) {
  thh::container_t<int> container;
  thh::handle_t handle = container.add();
  container.remove(handle);
  for (auto _ : state) {
    benchmark::DoNotOptimize(container.has(handle));
  }
}

BENCHMARK(HasElementNotPresent);

static void Resolve(benchmark::State& state) {
  thh::container_t<int> container;
  thh::handle_t handle = container.add();
  for (auto _ : state) {
    [[maybe_unused]] int* element = container.resolve(handle);
    benchmark::DoNotOptimize(element);
    benchmark::ClobberMemory();
  }
}

BENCHMARK(Resolve);

static void EnumerateCallback(benchmark::State& state) {
  thh::container_t<int> container;
  std::vector<thh::handle_t> handles;
  for (int i = 0; i < 10; ++i) {
    handles.push_back(container.add());
  }
  for (auto _ : state) {
    container.enumerate([i = 0](auto& element) mutable {
      element = i++;
      benchmark::ClobberMemory();
    });
  }
}

BENCHMARK(EnumerateCallback);

static void EnumerateCallbackResolve(benchmark::State& state) {
  thh::container_t<int> container;
  std::vector<thh::handle_t> handles;
  for (int i = 0; i < 10; ++i) {
    handles.push_back(container.add());
  }
  for (auto _ : state) {
    for (size_t i = 0; i < container.size(); ++i) {
      int* element = container.resolve(handles[i]);
      *element = i;
      benchmark::ClobberMemory();
    }
  }
}

BENCHMARK(EnumerateCallbackResolve);

BENCHMARK_MAIN();
