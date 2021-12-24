add_subdirectory(display)
add_subdirectory(gps)

target_include_directories(geeps PRIVATE
    display
    gps
)