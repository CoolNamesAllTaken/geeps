if(ON_HOST)
# Build for testing on host.
target_sources(geeps_test PRIVATE

)
else()
# Build for embedded target
target_sources(geeps PRIVATE
    geeps_gui.cc
    sd_utils.cpp
    scavenger_hunt.cpp
    
)
endif()