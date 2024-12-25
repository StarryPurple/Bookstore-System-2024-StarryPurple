cmake_minimum_required(VERSION 3.15)
project(Bookstore_StarryPurple)

set(CMAKE_CXX_STANDARD 20)

#set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/src
)

file(GLOB src_list CONFIGURE_DEPENDS src/*.cpp)

add_executable(code
        Main.cpp
        ${src_list}
)