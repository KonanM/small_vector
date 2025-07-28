cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(small_vector_tests
  LANGUAGES CXX
)

# ---- Options ----

option(ENABLE_TEST_COVERAGE "Enable test coverage" OFF)
option(TEST_INSTALLED_VERSION "Test the version found by find_package" OFF)

# ---- Dependencies ----

include(../cmake/CPM.cmake)

CPMAddPackage(
  NAME doctest
  GITHUB_REPOSITORY onqtam/doctest
  GIT_TAG v2.4.12
)

if (TEST_INSTALLED_VERSION)
  find_package(small_vector REQUIRED)
else()
  CPMAddPackage(
    NAME small_vector
    SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/..
  )
endif()

CPMAddPackage(
  NAME Format.cmake
  GITHUB_REPOSITORY TheLartians/Format.cmake
  VERSION 1.3
)

# ---- Create binary ----

file(GLOB sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)
add_executable(small_vector_tests ${sources})
target_link_libraries(small_vector_tests doctest small_vector)
set_target_properties(small_vector_tests PROPERTIES CXX_STANDARD 17)

# enable compiler warnings
if (NOT TEST_INSTALLED_VERSION)
  if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(small_vector_tests INTERFACE -Wall -Wextra -Wpedantic -Wno-c++98-compat -Wno-c++14-compat -Wno-missing-prototypes)
  elseif(MSVC)
    target_compile_options(small_vector_tests INTERFACE /W4 /Ob3 /Arch:AVX)
    target_compile_definitions(small_vector_tests PUBLIC DOCTEST_CONFIG_USE_STD_HEADERS)
  endif()
endif()

# ---- Add small_vectorTests ----

ENABLE_TESTING() 

# Note: doctest and similar testing frameworks can automatically configure CMake tests
# For other testing frameworks add the tests target instead:
# ADD_TEST(small_vector_tests small_vectorTests)

include(${doctest_SOURCE_DIR}/scripts/cmake/doctest.cmake)
doctest_discover_tests(small_vector_tests)

# ---- code coverage ----

if (ENABLE_TEST_COVERAGE)
  target_compile_options(small_vector_tests PUBLIC -O0 -g -fprofile-arcs -ftest-coverage)
  target_link_options(small_vector_tests PUBLIC -fprofile-arcs -ftest-coverage)
endif()
