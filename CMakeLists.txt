cmake_minimum_required(VERSION 3.20)
project(test_libraries C)

set(CMAKE_C_STANDARD 11)

add_library(test_libraries library.c)
