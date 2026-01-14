cmake_minimum_required(VERSION 3.16)
project(nhd12864_cpp LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GPIOD REQUIRED libgpiod)

add_library(nhd12864
    src/spi_linux.cpp
    src/gpio_gpiod.cpp
    src/st7565.cpp
    src/graphics.cpp
)
target_include_directories(nhd12864 PUBLIC include ${GPIOD_INCLUDE_DIRS})
target_link_libraries(nhd12864 PUBLIC ${GPIOD_LIBRARIES})
target_compile_options(nhd12864 PRIVATE -Wall -Wextra -Wpedantic)

add_executable(nhd12864_demo src/main_demo.cpp)
target_link_libraries(nhd12864_demo PRIVATE nhd12864)

add_executable(nhd12864_gpio src/main_gpio.cpp)
target_link_libraries(nhd12864_gpio PRIVATE nhd12864)
