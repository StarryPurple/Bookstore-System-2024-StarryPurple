cmake_minimum_required(VERSION 3.15)
project(Bookstore_StarryPurple)

set(CMAKE_CXX_STANDARD 20)

include_directories(${PROJECT_SOURCE_DIR}/include)

file(GLOB_RECURSE src_list src/*.cpp)

add_executable(code ${src_list})