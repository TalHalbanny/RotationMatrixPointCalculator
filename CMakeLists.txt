cmake_minimum_required(VERSION 3.16)
project(PointRotation LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(PointRotation src/main.cpp)
target_include_directories(PointRotation PRIVATE src)
target_link_libraries(PointRotation PRIVATE opengl32 glu32 user32 gdi32)

if(MINGW)
    target_link_options(PointRotation PRIVATE -municode -static)
endif()

if(MSVC)
    target_compile_definitions(PointRotation PRIVATE UNICODE _UNICODE NOMINMAX)
    target_link_options(PointRotation PRIVATE /ENTRY:wmainCRTStartup)
endif()
