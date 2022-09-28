if(CROSS_COMPILED)
# Build for testing on host.
target_sources(geeps_test PRIVATE
    geeps_gui.hh
)
else()
# Build for embedded target
target_sources(geeps PRIVATE
    geeps_gui.hh
)
endif()