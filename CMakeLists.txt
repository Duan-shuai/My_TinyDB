set(CMAKE_CXX_COMPILER "g++")
project(AntiDB
        LANGUAGES CXX
        )

cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_PROJECT_VERSION 1.0)


set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
set(BINARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")

include_directories(${CMAKE_SOURCE_DIR}/include)

add_subdirectory(src)




