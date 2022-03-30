cmake_minimum_required(VERSION 3.15)
project(thh-handle-vector LANGUAGES CXX)

include(FetchContent)

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(
  ${PROJECT_NAME}
  INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_17)

option(THH_HANDLE_ENABLE_TEST "Enable testing" OFF)
option(THH_HANDLE_ENABLE_BENCH "Enable benchmarking" OFF)

if (THH_HANDLE_ENABLE_TEST)
  FetchContent_Declare(
    doctest
    GIT_REPOSITORY https://github.com/onqtam/doctest.git
    GIT_TAG f0ab4e793fa863d4a5929ff263ddd3bebec8767e)
  FetchContent_MakeAvailable(doctest)
  add_executable(${PROJECT_NAME}-test)
  target_sources(${PROJECT_NAME}-test PRIVATE test.cpp)
  target_link_libraries(${PROJECT_NAME}-test ${PROJECT_NAME} doctest)
  target_compile_options(
    ${PROJECT_NAME}-test
    PRIVATE $<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/W4 /WX>
            $<$<COMPILE_LANG_AND_ID:CXX,AppleClang,Clang>:
            -Weverything -Wall -Wextra -pedantic -Wno-c++98-compat;-Wno-
            -Wno-sign-conversion;-Wno->
            $<$<COMPILE_LANG_AND_ID:CXX,GNU>:-Wall -Wextra -pedantic>)
endif()

if (THH_HANDLE_ENABLE_BENCH)
  set(BENCHMARK_ENABLE_TESTING OFF)
  FetchContent_Declare(
    benchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git)
  FetchContent_MakeAvailable(benchmark)
  add_executable(${PROJECT_NAME}-bench)
  target_sources(${PROJECT_NAME}-bench PRIVATE bench.cpp)
  target_link_libraries(${PROJECT_NAME}-bench ${PROJECT_NAME} benchmark)
  target_compile_options(
    ${PROJECT_NAME}-bench
    PRIVATE $<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/W4 /WX>
            $<$<COMPILE_LANG_AND_ID:CXX,AppleClang,Clang,GNU>:
            -Wall -Wextra -pedantic>)
endif()
