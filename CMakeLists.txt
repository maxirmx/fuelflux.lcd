cmake_minimum_required(VERSION 3.16)
project(nhd12864_cpp LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GPIOD REQUIRED libgpiod)
pkg_check_modules(FREETYPE REQUIRED freetype2)

# Base hardware library (low-level drivers)
add_library(nhd12864
    src/spi_linux.cpp
    src/gpio_gpiod.cpp
    src/st7565.cpp
    src/graphics.cpp
    src/ft_text.cpp
)
target_include_directories(nhd12864 PUBLIC include ${GPIOD_INCLUDE_DIRS} ${FREETYPE_INCLUDE_DIRS})
target_link_libraries(nhd12864 PUBLIC ${GPIOD_LIBRARIES} ${FREETYPE_LIBRARIES})
target_compile_options(nhd12864 PRIVATE -Wall -Wextra -Wpedantic)

# Four Line Display Library (reusable high-level API)
add_library(four_line_display
    src/four_line_display.cpp
)
target_include_directories(four_line_display PUBLIC include)
target_link_libraries(four_line_display PUBLIC nhd12864)
target_compile_options(four_line_display PRIVATE -Wall -Wextra -Wpedantic)

# Demo executables
add_executable(nhd12864_demo src/main_demo.cpp)
target_link_libraries(nhd12864_demo PRIVATE nhd12864)

add_executable(nhd12864_fourline_demo src/main_fourline_demo.cpp)
target_link_libraries(nhd12864_fourline_demo PRIVATE four_line_display nhd12864)

# Testing with GoogleTest
option(BUILD_TESTS "Build the tests" ON)

if(BUILD_TESTS)
    # Download and configure GoogleTest
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
    )
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    enable_testing()

    # Add test executable
    add_executable(test_four_line_display
        tests/test_four_line_display.cpp
    )
    target_link_libraries(test_four_line_display
        PRIVATE
        four_line_display
        nhd12864
        GTest::gtest
        GTest::gtest_main
    )

    # Discover tests
    include(GoogleTest)
    gtest_discover_tests(test_four_line_display)
endif()
