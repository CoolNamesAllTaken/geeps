if(CROSS_COMPILED)
# Build for testing on host.
target_sources(geeps_test PRIVATE
    epaper.cc
)
else()
# Build for embedded target
target_sources(geeps PRIVATE
    epaper.cc
)
endif()