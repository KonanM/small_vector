cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(small_vector_benchmark
  LANGUAGES CXX
)
include(../cmake/tools.cmake)

# ---- Options ----

option(ENABLE_TEST_COVERAGE "Enable test coverage" OFF)
option(TEST_INSTALLED_VERSION "Test the version found by find_package" OFF)

include(FetchContent)

set(gtest_force_shared_crt ON CACHE BOOL "Always use msvcrt.dll" FORCE)
# Download and unpack googletest at configure time

CPMAddPackage(
  NAME benchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  OPTIONS 
    "BENCHMARK_ENABLE_GTEST_TESTS OFF"
    "BENCHMARK_ENABLE_TESTING OFF"
    "BENCHMARK_ENABLE_INSTALL OFF"
  VERSION 1.5.0
)
# ---- Dependencies ----

include(../cmake/CPM.cmake)

if (TEST_INSTALLED_VERSION)
  find_package(small_vector REQUIRED)
else()
  CPMAddPackage(
    NAME small_vector
    SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/..
  )
endif()
# ---- Create library ----

add_executable(benchmark_small_vector benchmark_small_vector.cpp)
target_link_libraries(benchmark_small_vector PRIVATE small_vector benchmark)

# enable compiler warnings
if (NOT TEST_INSTALLED_VERSION)
  if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(benchmark_small_vector PRIVATE -Wall -Wextra -Wpedantic -Wno-c++98-compat -Wno-c++14-compat -Wno-missing-prototypes -Wno-global-constructors -march=native)
  elseif(MSVC)
    target_compile_options(benchmark_small_vector PRIVATE /W4 /Ob3 /GL /arch:AVX)
    target_link_options(benchmark_small_vector PRIVATE /LTCG)
  endif()
endif()
