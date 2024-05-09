cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(kaleidoscope LANGUAGES CXX)

add_executable(kaleidoscope ast.h driver.cpp lexer.h lexer.cpp log.h parser.cpp parser.h)