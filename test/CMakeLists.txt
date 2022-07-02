include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

add_executable(pdargs_test test.cc)
target_link_libraries(pdargs_test PRIVATE gtest_main)
target_include_directories(pdargs_test PRIVATE ${CMAKE_SOURCE_DIR}/include)

include(GoogleTest)

gtest_discover_tests(pdargs_test)

