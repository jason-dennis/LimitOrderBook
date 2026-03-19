cmake_minimum_required(VERSION 3.20)
project(LimitOrderBook)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "-O0 -g")

include(FetchContent)

set(FETCHCONTENT_UPDATES_DISCONNECTED ON)
set(FETCHCONTENT_QUIET OFF)

# --- Fetch FTXUI --------------------------------------------------------------
FetchContent_Declare(ftxui
        GIT_REPOSITORY https://github.com/arthursonzogni/ftxui.git
        GIT_TAG        v5.0.0
        GIT_PROGRESS   TRUE
        GIT_SHALLOW    TRUE
        EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(ftxui)

# --- Fetch GoogleTest ---------------------------------------------------------
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

# --- Fetch GoogleBenchmark ----------------------------------------------------
FetchContent_Declare(
        googlebenchmark
        URL https://github.com/google/benchmark/archive/refs/tags/v1.8.3.tar.gz
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest googlebenchmark)

# --- Common sources -----------------------------------------------------------
set(COMMON_SOURCES
        src/Storage/MultisetOrderBookStorage.cpp
        src/Engine/MatchingEngine.cpp
        src/Engine/CoreEngine.cpp
        src/Engine/AppEngine.cpp
        src/Domain/order.cpp
        src/Storage/BinaryOrderBookStorage.cpp
)

add_library(trading_lib STATIC ${COMMON_SOURCES})
target_include_directories(trading_lib PUBLIC include)

add_library(trading_lib_cov STATIC ${COMMON_SOURCES})
target_include_directories(trading_lib_cov PUBLIC include)

# --- Main executable ----------------------------------------------------------
add_executable(LimitOrderBook main.cpp)
target_include_directories(LimitOrderBook PRIVATE include)
target_link_libraries(LimitOrderBook
        PRIVATE trading_lib
        PRIVATE ftxui::screen
        PRIVATE ftxui::dom
        PRIVATE ftxui::component
)

# --- Tests --------------------------------------------------------------------
enable_testing()
include(GoogleTest)

add_executable(run_tests
        Tests/Storage/test_multiset_storage.cpp
        Tests/Domain/test_order.cpp
        Tests/Engine/TestMatchingEngine.cpp
        Tests/Storage/test_binaryorderbook.cpp
        Tests/Engine/TestAppEngine.cpp
        Tests/Engine/TestCoreEngine.cpp
)
target_link_libraries(run_tests PRIVATE trading_lib_cov gtest_main)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(trading_lib_cov PRIVATE --coverage)
    target_link_options(trading_lib_cov PRIVATE --coverage)
    target_compile_options(run_tests PRIVATE --coverage)
    target_link_options(run_tests PRIVATE --coverage)
endif()

gtest_discover_tests(run_tests)

# --- Benchmarks ---------------------------------------------------------------
add_executable(run_benchmarks
        Tests/benchmarks/OrderBookBenchmark.cpp
)
target_link_libraries(run_benchmarks PRIVATE trading_lib benchmark::benchmark_main)


# --- Emscripten ---------------------------------------------------------------
if (EMSCRIPTEN)
    string(APPEND CMAKE_CXX_FLAGS " -s USE_PTHREADS")
    string(APPEND CMAKE_EXE_LINKER_FLAGS " -s ASYNCIFY")
    string(APPEND CMAKE_EXE_LINKER_FLAGS " -s PROXY_TO_PTHREAD")

    foreach(file "index.html" "run_webassembly.py")
        configure_file("src/${file}" ${file})
    endforeach(file)
endif()