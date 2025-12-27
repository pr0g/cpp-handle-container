cmake_minimum_required(VERSION 3.15)
project(thh-handle-vector LANGUAGES CXX)

include(FetchContent)
include(GNUInstallDirs)

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(
  ${PROJECT_NAME}
  INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_17)

option(THH_HANDLE_ENABLE_TEST "Enable testing" OFF)
option(THH_HANDLE_ENABLE_BENCH "Enable benchmarking" OFF)

if(THH_HANDLE_ENABLE_TEST)
  FetchContent_Declare(
    doctest
    GIT_REPOSITORY https://github.com/onqtam/doctest.git
    GIT_TAG 1da23a3e8119ec5cce4f9388e91b065e20bf06f5)
  FetchContent_MakeAvailable(doctest)
  add_executable(${PROJECT_NAME}-test)
  target_sources(${PROJECT_NAME}-test PRIVATE test.cpp)
  target_link_libraries(${PROJECT_NAME}-test ${PROJECT_NAME} doctest)
  target_compile_options(
    ${PROJECT_NAME}-test
    PRIVATE $<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/W4
            /WX>
            $<$<COMPILE_LANG_AND_ID:CXX,AppleClang,Clang>:
            -Weverything
            -Wall
            -Wextra
            -pedantic
            -Wno-c++98-compat;-Wno-
            -Wno-sign-conversion;-Wno-
            -Wno-padded;-Wno->
            $<$<COMPILE_LANG_AND_ID:CXX,GNU>:-Wall
            -Wextra
            -pedantic>)
endif()

if(THH_HANDLE_ENABLE_BENCH)
  set(BENCHMARK_ENABLE_TESTING OFF)
  FetchContent_Declare(
    benchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG 04ccbd86038796c319ea19987457e651a24f6b44)
  FetchContent_MakeAvailable(benchmark)
  add_executable(${PROJECT_NAME}-bench)
  target_sources(${PROJECT_NAME}-bench PRIVATE bench.cpp)
  target_link_libraries(${PROJECT_NAME}-bench ${PROJECT_NAME} benchmark)
  target_compile_options(
    ${PROJECT_NAME}-bench
    PRIVATE $<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/W4 /WX>
            $<$<COMPILE_LANG_AND_ID:CXX,AppleClang,Clang,GNU>: -Wall -Wextra
            -pedantic>)
endif()

install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}-config)

install(
  EXPORT ${PROJECT_NAME}-config
  NAMESPACE ${PROJECT_NAME}::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})

install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/include/${PROJECT_NAME}/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME})
