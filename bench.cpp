#include "criterion/criterion.hpp"
#include "thh_handles/thh_handles.hpp"

BENCHMARK(AddElement)
{
  SETUP_BENCHMARK(
    thh::container_t<int> container;
  )
 
  // benchmarking
  const thh::handle_t handle = container.add();
  
  TEARDOWN_BENCHMARK(
    container.remove(handle);
  )
}

BENCHMARK(AddElementWithReserve)
{
  SETUP_BENCHMARK(
    thh::container_t<int> container;
    container.reserve(8);
  )
 
  // benchmarking
  const thh::handle_t handle = container.add();
  
  TEARDOWN_BENCHMARK(
    container.remove(handle);
  )
}

BENCHMARK(RemoveElement)
{
  SETUP_BENCHMARK(
    thh::container_t<int> container;
    const thh::handle_t handle = container.add();
  )
 
  // benchmarking
  container.remove(handle);
  
  TEARDOWN_BENCHMARK(
    // nothing
  )
}

BENCHMARK(HasElementPresent)
{
  SETUP_BENCHMARK(
    thh::container_t<int> container;
    thh::handle_t handle = container.add();
  )
 
  // benchmarking
  container.has(handle);
  
  TEARDOWN_BENCHMARK(
    container.remove(handle);
  )
}

BENCHMARK(HasElementNotPresent)
{
  SETUP_BENCHMARK(
    thh::container_t<int> container;
    thh::handle_t handle = container.add();
    container.remove(handle);
  )
 
  // benchmarking
  container.has(handle);
  
  TEARDOWN_BENCHMARK(
    // nothing
  )
}

BENCHMARK(Resolve)
{
  SETUP_BENCHMARK(
    thh::container_t<int> container;
    thh::handle_t handle = container.add();
  )
 
  // benchmarking
  [[maybe_unused]] int* element = container.resolve(handle);
  
  TEARDOWN_BENCHMARK(
    container.remove(handle);
  )
}

BENCHMARK(EnumerateCallback)
{
  SETUP_BENCHMARK(
    thh::container_t<int> container;
    std::vector<thh::handle_t> handles;
    for (int i = 0; i < 10; ++i) {
      handles.push_back(container.add());
    }
  )

  container.enumerate([i = 0](auto& element) mutable {
    element = i++;
  });

  TEARDOWN_BENCHMARK(
    for (size_t i = 0; i < container.size(); ++i) {
      container.remove(handles[i]);
    }
  )
}

BENCHMARK(EnumerateResolve)
{
  SETUP_BENCHMARK(
    thh::container_t<int> container;
    std::vector<thh::handle_t> handles;
    for (int i = 0; i < 10; ++i) {
      handles.push_back(container.add());
    }
  )

  for (size_t i = 0; i < container.size(); ++i) {
    int* element = container.resolve(handles[i]);
    *element = i;
  }

  TEARDOWN_BENCHMARK(
    for (size_t i = 0; i < container.size(); ++i) {
      container.remove(handles[i]);
    }
  )
}

CRITERION_BENCHMARK_MAIN()
