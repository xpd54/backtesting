cmake_minimum_required(VERSION 3.26)
project(trader)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# Set so binary gets created into build/ folder
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

file(GLOB base_src
  "src/base/*cpp"
  "src/common_interface/*cpp"
  "src/price_history/*cpp"
)
include_directories(
  src/**/
)

add_library(base STATIC ${base_src})
add_subdirectory(external/common_util)
add_subdirectory(data_generator)