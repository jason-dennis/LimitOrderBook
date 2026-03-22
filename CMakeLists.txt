cmake_minimum_required(VERSION 3.20)
project(LimitOrderBook)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "-O0 -g")
set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -static")
include(FetchContent)

# --- Fetch GLFW ---------------------------------------
FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG        3.4
)
set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

# --- Fetch GLFW ---------------------------------------
FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG        v1.91.8-docking
)
FetchContent_MakeAvailable(imgui)
# --- Fetch GLFW ---------------------------------------
# ImPlot (charts)
FetchContent_Declare(
        implot
        GIT_REPOSITORY https://github.com/epezent/implot.git
        GIT_TAG        v0.16
)
FetchContent_MakeAvailable(implot)

# ══════════════════════════════════════════
# ImGui library (sources + backends)
# ══════════════════════════════════════════

add_library(imgui_lib STATIC
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
        ${implot_SOURCE_DIR}/implot.cpp
        ${implot_SOURCE_DIR}/implot_items.cpp
        include/UI/OrderBookPanel.h
        include/UI/GraphPanel.h
        include/UI/OrderEntryPanel.h
        include/UI/OrdersPanel.h
)

target_include_directories(imgui_lib PUBLIC
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
        ${implot_SOURCE_DIR}
)

target_link_libraries(imgui_lib PUBLIC glfw)

# OpenGL
find_package(OpenGL REQUIRED)
target_link_libraries(imgui_lib PUBLIC OpenGL::GL)



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
        src/UI/TradeTerminal.cpp
)

add_library(trading_lib STATIC ${COMMON_SOURCES}
        src/UI/OrderBookPanel.cpp
        src/UI/GraphPanel.cpp
        src/UI/OrderEntryPanel.cpp
        src/UI/OrdersPanel.cpp)
target_include_directories(trading_lib PUBLIC include)

add_library(trading_lib_cov STATIC ${COMMON_SOURCES})
target_include_directories(trading_lib_cov PUBLIC include)
target_link_libraries(trading_lib_cov PUBLIC imgui_lib)

# --- Main executable ----------------------------------------------------------
add_executable(LimitOrderBook main.cpp)
target_include_directories(LimitOrderBook PRIVATE include)
target_link_libraries(LimitOrderBook PRIVATE trading_lib imgui_lib)
target_link_libraries(trading_lib PUBLIC imgui_lib)

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
target_link_libraries(run_tests PRIVATE trading_lib_cov gtest_main )

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