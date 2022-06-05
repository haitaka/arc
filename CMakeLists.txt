cmake_minimum_required(VERSION 3.21)
project(arc)

set(CMAKE_CXX_STANDARD 17)

add_executable(arc main.cpp object.cpp ir.cpp interpreter.cpp reference.cpp preparation.cpp preparation.h lexer.cpp lexer.h parser.cpp parser.h)
