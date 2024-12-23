cmake_minimum_required(VERSION 3.15)
project(Bookstore_StarryPurple)

set(CMAKE_CXX_STANDARD 20)

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/src)

#file(GLOB_RECURSE src_list src/*.cpp)

add_executable(code
        Main.cpp
        src/BookStoreExceptions.cpp
        src/filestream.cpp
        src/lrucache.cpp
        src/blocklist.cpp
        src/fmultimap.cpp
        src/validator.cpp)