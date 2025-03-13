cmake_minimum_required(VERSION 3.13)

# Add subdirectories after creating the target so that CMake doesn't get upset.
add_subdirectory(/geeps/firmware/src firmware/src) # maps firmware src folder to local firmware/src
add_subdirectory(src)
add_subdirectory(inc)

# In case there are files directly in src and inc
target_include_directories(geeps_test PRIVATE 
    src
    inc
)