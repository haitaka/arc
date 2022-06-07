cmake_minimum_required(VERSION 3.21)
project(arc)

set(CMAKE_CXX_STANDARD 17)

add_executable(arc main.cpp ast.cpp interpreter.cpp mm.cpp preparation.cpp lexer.cpp parser.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread -fPIE -pie -g")
