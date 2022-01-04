if(CROSS_COMPILED)
# Build for testing on host.
target_sources(geeps_test PRIVATE
    nmea_utils.cc
    pa1616s.cc
)
else()
# Build for embedded target
target_sources(geeps PRIVATE
    nmea_utils.cc
    pa1616s.cc
)
endif()