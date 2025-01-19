cmake_minimum_required(VERSION 3.26)
project(ohlc_generator)
set(CMAKE_CXX_STANDARD 17)

file(GLOB data_generator_src
        "src/*.cpp"
        "src/util/*.cpp"
)
add_executable(${PROJECT_NAME} ${data_generator_src})
target_link_libraries(${PROJECT_NAME} PRIVATE common_util)