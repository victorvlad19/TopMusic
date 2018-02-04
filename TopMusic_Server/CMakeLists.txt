cmake_minimum_required(VERSION 3.8)
project(TopMusic_Server)

add_compile_options(-l sqlite3)
set(CMAKE_CXX_STANDARD 14)

set(SOURCE_FILES main.cpp)
add_executable(TopMusic_Server ${SOURCE_FILES})

target_link_libraries(TopMusic_Server LINK_PUBLIC sqlite3)