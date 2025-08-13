cmake_minimum_required(VERSION 3.26)
project(plot)
set(CMAKE_CXX_STANDARD 17)

file(GLOB plot_src
        "*.cpp"
)
add_executable(${PROJECT_NAME} ${plot_src})
target_link_libraries(${PROJECT_NAME} PRIVATE 
        common_util 
 )