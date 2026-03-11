cmake_minimum_required(VERSION 3.20)
project(LimitOrderBook)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# --- Google Test ---
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

# --- Google Benchmark ---
FetchContent_Declare(
        googlebenchmark
        URL https://github.com/google/benchmark/archive/refs/tags/v1.8.3.tar.gz
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest googlebenchmark)

# --- Sursă comună pentru a evita duplicarea ---
set(SOURCE_FILES
        src/Domain/order.cpp
        src/Storage/MultisetOrderBookStorage.cpp
        include/Domain/trade.h
        include/Engine/MatchingEngine.h
        src/Engine/MatchingEngine.cpp
)

# --- Executabilul Principal ---
add_executable(LimitOrderBook main.cpp ${SOURCE_FILES}
        include/Domain/trade.h
        include/Engine/MatchingEngine.h
        src/Engine/MatchingEngine.cpp)
target_include_directories(LimitOrderBook PRIVATE include)

# --- Unit Tests ---
enable_testing()
add_executable(run_tests
        Tests/Domain/test_order.cpp
        Tests/Storage/test_multiset_storage.cpp
        ${SOURCE_FILES}
)
target_include_directories(run_tests PRIVATE include)
target_link_libraries(run_tests gtest_main)

# --- Benchmarks ---
add_executable(run_benchmarks
        Tests/benchmarks/OrderBookBenchmark.cpp
        ${SOURCE_FILES}
)
target_include_directories(run_benchmarks PRIVATE include)
# Aici era greșeala: trebuie să legi la run_benchmarks
target_link_libraries(run_benchmarks benchmark::benchmark_main)

# --- Coverage ---
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(run_tests PRIVATE --coverage)
    target_link_options(run_tests PRIVATE --coverage)
endif()

include(GoogleTest)
gtest_discover_tests(run_tests)