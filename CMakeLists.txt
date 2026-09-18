cmake_minimum_required(VERSION 3.20)

project(even_flop
  VERSION 0.1.0
  DESCRIPTION "Find the flop that splits equity closest to 50/50 between two preflop hands"
  LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

option(EVEN_FLOP_BUILD_TESTS "Build even-flop unit tests" ON)

include(FetchContent)

# --- phevaluator: perfect-hash 5-7 card evaluator (Apache-2.0) -----------------
# Its CMakeLists lives in cpp/ and defaults BUILD_TESTS=ON; force it off so we
# don't build the upstream test suite as part of our tree.
set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_Declare(phevaluator
  GIT_REPOSITORY https://github.com/HenryRLee/PokerHandEvaluator.git
  GIT_TAG        v0.6.0
  GIT_SHALLOW    TRUE
  SOURCE_SUBDIR  cpp
)
FetchContent_MakeAvailable(phevaluator)

# --- core library -------------------------------------------------------------
add_library(even_flop_core
  src/card.cpp
  src/hand.cpp
  src/deck.cpp
  src/equity.cpp
  src/search.cpp
)
target_include_directories(even_flop_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(even_flop_core PUBLIC pheval)
target_compile_options(even_flop_core PRIVATE -Wall -Wextra -Wpedantic)

find_package(Threads REQUIRED)
target_link_libraries(even_flop_core PUBLIC Threads::Threads)

# --- cli ----------------------------------------------------------------------
# Argument parsing and formatting live in a library rather than in the
# executable so the tests can link and exercise them directly.
add_library(even_flop_cli
  src/cli/options.cpp
  src/cli/report.cpp
)
target_include_directories(even_flop_cli PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/cli)
target_link_libraries(even_flop_cli PUBLIC even_flop_core)
target_compile_options(even_flop_cli PRIVATE -Wall -Wextra -Wpedantic)

add_executable(even-flop src/cli/main.cpp)
target_link_libraries(even-flop PRIVATE even_flop_cli)
target_compile_options(even-flop PRIVATE -Wall -Wextra -Wpedantic)

# --- tests --------------------------------------------------------------------
if(EVEN_FLOP_BUILD_TESTS)
  enable_testing()
  FetchContent_Declare(googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.18.0
    GIT_SHALLOW    TRUE
  )
  set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)
  add_subdirectory(tests)
endif()
