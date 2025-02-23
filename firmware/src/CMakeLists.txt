add_subdirectory(display)
add_subdirectory(gps)
add_subdirectory(app)

if(ON_HOST)
# Build for testing on host.
target_include_directories(geeps_test PRIVATE
    display
    gps
    app
)
# Don't include main for testing.
else()
# Build for embedded target
target_include_directories(geeps PRIVATE
    display
    gps
    app
)
target_sources(geeps
PRIVATE
    main.cpp
    bsp.cpp
)
endif()