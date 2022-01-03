add_subdirectory(display)
add_subdirectory(gps)

if(CROSS_COMPILED)
# Build for testing on host.
target_include_directories(geeps_test PRIVATE
    display
    gps
)
else()
# Build for embedded target
target_include_directories(geeps PRIVATE
    display
    gps
)
endif()