# Building Test target only, This doesn't build project target which have main.cpp
project(hello_test)

# List out UnitTest files for adding as executable
file(GLOB test_src
  "test/*.cpp"
  "test/**/*.cpp"
)

# List out all module/class/function files which UnitTest has been written
file(GLOB testing_src
  "../src/**/*.cpp"
)

# Include them so can access their header files
include_directories(
  ../src/util 
  ../src/Stack 
  ../src/Turtle 
  ../src/Painter
  )

# Define these module/class/function as a library so it can be linked togather with Test target Ex:- hello_test
add_library(testing_lib ${testing_src})
add_executable(
  ${PROJECT_NAME}
  ${test_src}
)

# Set GMOCK build ON
set(BUILD_GMOCK ON)

# Link gtest and created lib which have all the function/module/class to be tested
# As this also build gmock target that lib as well.
target_link_libraries(
  ${PROJECT_NAME}
  GTest::gtest_main
  gmock
  testing_lib
)

# Add googletest subdirectory. When this line get build it will jump to googletest CMakeLists to build
add_subdirectory(googletest)

# Include it so it can be run by ctest command
include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME})