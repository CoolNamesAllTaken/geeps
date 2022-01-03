cmake_minimum_required(VERSION 3.13)

project(geeps_test_project C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

add_compile_options(
    -Wall
    -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
    -Wno-unused-function # we have some for the docs that aren't called
    -Wno-maybe-uninitialized
)

file (GLOB gtest_src CONFIGURE_DEPENDS "src/*.h" "src/*.cpp")

add_executable(geeps_test ${gtest_src})

# Add subdirectories after creating the target so that CMake doesn't get upset.
#add_subdirectory(src)
#add_subdirectory(inc)

target_include_directories(geeps_test PRIVATE 
    src
    inc
    ../firmware/src
    ../firmware/inc
)

# Pull in google test library
add_library(libgtest SHARED IMPORTED)

# Provide the full path to the library, so CMake knows where to find it.
set_target_properties(libgtest PROPERTIES IMPORTED_LOCATION /root/modules/googletest/lib/libgtest.so)
target_link_libraries(geeps_test PRIVATE libgtest)