cmake_minimum_required(VERSION 3.26)
project(data_generator)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# Set so binary gets created into build/ folder
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

file(GLOB data_generator_SRC
        "DataGenerator/src/*.cpp"
        "DataGenerator/src/util/*.cpp"
)
add_executable(data_generator ${data_generator_SRC})
add_subdirectory(external/common_util)
target_link_libraries(${PROJECT_NAME} PRIVATE common_util)