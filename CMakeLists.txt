cmake_minimum_required(VERSION 3.19)

project(pdargs)

set(CMAKE_CXX_STANDARD 17)

enable_testing()

include(cmake/find_gtest.cmake)

add_executable(
    main
    main.cc
)

target_compile_options(main PRIVATE "-Wall -Wextra")
target_include_directories(main PRIVATE include)

add_compile_options("-fsanitize=undefined")
add_link_options("-fsanitize=undefined")

#include(GoogleTest)
#gtest_discover_tests(hello_test)

