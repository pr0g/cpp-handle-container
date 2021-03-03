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

CRITERION_BENCHMARK_MAIN()
