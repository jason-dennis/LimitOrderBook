cmake_minimum_required(VERSION 3.20)
project(LimitOrderBook)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(LimitOrderBook main.cpp
        src/Domain/order.cpp
        include/Domain/order.h)
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(--coverage)
    add_link_options(--coverage)
endif()

enable_testing()
add_executable(run_tests Tests/Domain/test_order.cpp src/Domain/order.cpp)
target_include_directories(run_tests PRIVATE Domain)
target_include_directories(LimitOrderBook PRIVATE Domain)

target_link_libraries(run_tests gtest_main)

include(GoogleTest)
gtest_discover_tests(run_tests)