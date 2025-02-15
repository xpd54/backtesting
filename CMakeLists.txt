cmake_minimum_required(VERSION 3.26)
project(trade_simulator)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# Set so binary gets created into build/ folder
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
# set(CMAKE_BUILD_TYPE "Debug")

add_compile_options("-O3;-march=native;-fomit-frame-pointer;")

add_subdirectory(external/common_util)
include_directories(
  backtesting/base/
  external/common_util/include
)

file(GLOB base_src
  "backtesting/base/**/*.cpp"
)
add_library(base STATIC ${base_src})
add_subdirectory(data_generator)
add_subdirectory(result_plot)

file(GLOB simulator_src
  "backtesting/*.cpp"
  "backtesting/simulators/**/*.cpp"
  "backtesting/simulators/*.cpp"
  "backtesting/execution/*.cpp"
  "backtesting/logs/*.cpp"
)

add_executable(${PROJECT_NAME} ${simulator_src} ${base_src})
target_link_libraries(${PROJECT_NAME} 
  PRIVATE 
  common_util 
  base
 )