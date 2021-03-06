# Experimental handle container in C++

## Overview

Following on from [c-handle-container](https://github.com/pr0g/c-handle-container), this library builds on the same ideas but supports a dynamic number of elements without a fixed capacity and is templated so arbitrary types can stored.

Excellent resources on the subject:

- [Managing Decoupling Part 4 -- The ID Lookup Table](http://bitsquid.blogspot.com/2011/09/managing-decoupling-part-4-id-lookup.html) - Niklas Gray ([Twitter](https://twitter.com/niklasfrykholm), [GitHub](https://github.com/niklas-ourmachinery))
- [Managing Data Relationships](https://gamesfromwithin.com/managing-data-relationships) - Noel Llopis ([Twitter](https://twitter.com/noel_llopis), [GitHub](https://github.com/llopis))
- [Handles are the better pointers](https://floooh.github.io/2018/06/17/handles-vs-pointers.html) - Andre Weissflog (FlohOfWoe, Floooh) ([Twitter](https://twitter.com/FlohOfWoe), [GitHub](https://github.com/floooh))

## Building

When building the tests and benchmarks, `-DBENCHMARK_ENABLE_TESTING=OFF` must be passed to CMake at configure time to ensure the Google Test dependency on Google Benchmark is not required.
