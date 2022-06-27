cmake_minimum_required(VERSION 3.21)
project(arc)

set(CMAKE_CXX_STANDARD 17)

set(SRC ast.cpp interpreter.cpp mm.cpp preparation.cpp parsing/lexer.cpp parsing/parser.cpp logger.cpp logger.h run.cpp)
add_executable(arc main.cpp ${SRC})

