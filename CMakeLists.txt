cmake_minimum_required(VERSION 3.16)
project(Bookstore)

set(CMAKE_CXX_STANDARD 20)
add_compile_options(-std=c++2a -g )

add_executable(code 
${PROJECT_SOURCE_DIR}/src/main.cpp)


