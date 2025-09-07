cmake_minimum_required(VERSION 3.25)
project(chip8)

set(CMAKE_CXX_STANDARD 17)

set(SDL2_INCLUDE_DIR C:/SDL/include)
set(SDL2_LIB_DIR C:/SDL/lib/x64)

include_directories(${SDL2_INCLUDE_DIR})
link_directories(${SDL2_LIB_DIR})

add_executable(chip8 main.cpp chip8.hpp chip8.cpp)

target_link_libraries(${PROJECT_NAME} SDL2main SDL2)