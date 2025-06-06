cmake_minimum_required(VERSION 3.30)
project(windows_lab)

set(CMAKE_CXX_STANDARD 20)
add_definitions(-DUNICODE -D_UNICODE)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mwindows")

add_executable(windows_lab main.cpp
        basewin.h
        scene.h)
target_link_libraries(windows_lab PRIVATE d2d1)
