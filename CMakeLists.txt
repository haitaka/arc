cmake_minimum_required(VERSION 3.21)
project(arc)

set(CMAKE_CXX_STANDARD 17)

set(SRC ast.cpp interpreter.cpp mm.cpp preparation.cpp parsing/lexer.cpp parsing/parser.cpp logger.cpp logger.h run.cpp)
add_executable(arc main.cpp ${SRC})

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/15460959cbbfa20e66ef0b5ab497367e47fc0a04.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()
include(GoogleTest)

add_executable(test_core ${SRC} tests/helper.cpp tests/core.cpp)
target_link_libraries(test_core gtest_main gmock)
gtest_discover_tests(test_core)

add_executable(test_concurrent ${SRC} tests/helper.cpp tests/concurrent.cpp)
target_link_libraries(test_concurrent gtest_main gmock)
gtest_discover_tests(test_concurrent)
target_compile_options(test_concurrent PRIVATE -fPIE PRIVATE -g PRIVATE -fsanitize=thread)
target_link_options(test_concurrent PRIVATE -fsanitize=thread PRIVATE -pie)
