cmake_minimum_required(VERSION 3.26)
project(data_generator)
set(CMAKE_CXX_STANDARD 17)
file(GLOB data_generator_SRC
        "src/*.cpp"
        "src/util/*.cpp"
)
add_executable(data_generator ${data_generator_SRC})
add_subdirectory(external/common_util)
target_link_libraries(${PROJECT_NAME} PRIVATE common_util)