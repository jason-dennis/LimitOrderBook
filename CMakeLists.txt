cmake_minimum_required(VERSION 3.20)
project(LimitOrderBook)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

FetchContent_Declare(
        googlebenchmark
        URL https://github.com/google/benchmark/archive/refs/tags/v1.8.3.tar.gz
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest googlebenchmark)

set(COMMON_SOURCES
        src/Storage/MultisetOrderBookStorage.cpp
        src/Engine/MatchingEngine.cpp
        src/Domain/order.cpp
)

add_library(trading_lib STATIC ${COMMON_SOURCES})
target_include_directories(trading_lib PUBLIC include)

add_executable(LimitOrderBook main.cpp
        src/Domain/order.cpp)
target_link_libraries(LimitOrderBook PRIVATE trading_lib)

enable_testing()
include(GoogleTest)

add_executable(run_tests
        Tests/Storage/test_multiset_storage.cpp
        Tests/Engine/TestMatchingEngine.cpp
        src/Domain/order.cpp
)
target_link_libraries(run_tests PRIVATE trading_lib gtest_main)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(run_tests PRIVATE --coverage)
    target_link_options(run_tests PRIVATE --coverage)
endif()

gtest_discover_tests(run_tests)

add_executable(run_benchmarks
        Tests/benchmarks/OrderBookBenchmark.cpp
)
target_link_libraries(run_benchmarks PRIVATE trading_lib benchmark::benchmark_main)