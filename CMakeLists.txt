cmake_minimum_required(VERSION 3.1)
project(eclipse)

#set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
set(BUILD_SHARED_LIBS OFF)

find_package(OpenCL REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)
find_package(OpenMP REQUIRED)
find_package(CURL REQUIRED)
find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)

# Set compilation flags
set(CMAKE_CXX_STANDARD 14)
set(CMAKE CXX STANDARD REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS} -std=c++1y -O3 -Wall -Wextra -Wpedantic -march=native")

#add_definitions(-DDEBUG)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

include_directories(OpenCL_INCLUDE_DIRS)
include_directories(OPENGL_INCLUDE_DIR)
include_directories(GLEW_INCLUDE_DIRS)
include_directories(GLFW_INCLUDE_DIRS)
include_directories(CURL_INCLUDE_DIRS)

include_directories(src)
add_subdirectory(src/eclipse)
