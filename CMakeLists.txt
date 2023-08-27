cmake_minimum_required(VERSION 3.22.0)

set(PROJECT vulkantutorial)
project(${PROJECT})

# Define target names
set(PROJECT_LIB ${PROJECT})
set(PROJECT_EXE ${PROJECT}.bin)

# Build library and executable
add_subdirectory(src)

