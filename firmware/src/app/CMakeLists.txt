if(CROSS_COMPILED)
# Build for testing on host.
target_sources(geeps_test PRIVATE

)
else()
# Build for embedded target
target_sources(geeps PRIVATE
    geeps_gui.cc
)
endif()