cmake_minimum_required(VERSION 3.15)
project(thh_handles LANGUAGES CXX)

include(FetchContent)
FetchContent_Declare(
  doctest
  GIT_REPOSITORY https://github.com/onqtam/doctest.git
  GIT_TAG f0ab4e793fa863d4a5929ff263ddd3bebec8767e)
FetchContent_Declare(
  benchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git)
FetchContent_MakeAvailable(doctest benchmark)

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(
  ${PROJECT_NAME}
  INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_17)

add_executable(${PROJECT_NAME}_test)
target_sources(${PROJECT_NAME}_test PRIVATE test.cpp)
target_link_libraries(${PROJECT_NAME}_test ${PROJECT_NAME} doctest)

add_executable(${PROJECT_NAME}_bench)
target_sources(${PROJECT_NAME}_bench PRIVATE bench.cpp)
target_link_libraries(${PROJECT_NAME}_bench ${PROJECT_NAME} benchmark)
