# Experimental handle container in C++

## Overview

Following on from [c-handle-container](https://github.com/pr0g/c-handle-container), this library builds on the same ideas but supports a dynamic number of elements without a fixed capacity and is templated so arbitrary types can stored.

> _Note: This is an early draft, proof-of-concept experiment. There's probably bugs, oversights and problems I haven't yet thought of/ran into. Please treat as a reference implementation. YMMV_ 🙂

Excellent resources on the subject:

- [Managing Decoupling Part 4 -- The ID Lookup Table](http://bitsquid.blogspot.com/2011/09/managing-decoupling-part-4-id-lookup.html) - Niklas Gray ([Twitter](https://twitter.com/niklasfrykholm), [GitHub](https://github.com/niklas-ourmachinery))
- [Managing Data Relationships](https://gamesfromwithin.com/managing-data-relationships) - Noel Llopis ([Twitter](https://twitter.com/noel_llopis), [GitHub](https://github.com/llopis))
- [Handles are the better pointers](https://floooh.github.io/2018/06/17/handles-vs-pointers.html) - Andre Weissflog (FlohOfWoe, Floooh) ([Twitter](https://twitter.com/FlohOfWoe), [GitHub](https://github.com/floooh))

## Building

To build tests and benchmarks pass `-DTHH_HANDLE_ENABLE_TEST` and `-DTHH_HANDLE_ENABLE_BENCH` to CMake.

```bash
cmake -B build -DTHH_HANDLE_ENABLE_TEST=ON -DTHH_HANDLE_ENABLE_BENCH=ON
```

> Note: Depending on the generator, use `-DCMAKE_BUILD_TYPE=Release` for the benchmarks (or build with `--config Release` if using a multi-config generator).

Note: `-DBENCHMARK_ENABLE_TESTING=OFF` is passed to CMake at configure time to ensure the Google Test dependency on Google Benchmark is not required (already set inside `CMakeLists.txt`).

## Gotchas

The `resolve` function (added in the initial version of the library) was easy to use incorrectly due to the fact that if the internal vector had to grow and reallocate, any existing pointers would be invalidated (dangling).

This was unfortunately quite easy to do by mistake. A much better interface which makes this harder to do is provided by `call`. This accepts a handle and a callable object (a lambda taking an element as its only parameter) which is resolved internally and called. This makes it much harder to accidentally hold onto a pointer for too long (see the tests for examples).

## Usage

Either drop the `thh_handles` inside `include/` into your project (and then just `#include "thh_handles/thh_handles.hpp"`) or use CMake's `FetchContent` command.

e.g.

```cmake
# CMakeLists.txt
include(FetchContent)
FetchContent_Declare(
  thh_handles
  GIT_REPOSITORY https://github.com/pr0g/cpp-handle-container.git
  GIT_TAG        <latest-commit>)
FetchContent_MakeAvailable(thh_handles)
...
target_link_libraries(<your-project> <PRIVATE/PUBLIC> thh_handles)
```

```c++
// .h/cpp file
#include "thh_handles/thh_handles.hpp"
```
