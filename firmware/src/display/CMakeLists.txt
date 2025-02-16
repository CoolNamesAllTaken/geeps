if(CROSS_COMPILED)
# Build for testing on host.
target_sources(geeps_test PRIVATE

)
else()
# Build for embedded target
target_sources(geeps PRIVATE
    epaper.cc
    hV_Colours565.cpp
    hV_Font_Terminal.cpp
    hV_Screen_Buffer.cpp
    hV_Utilities.cpp
    Screen_EPD_EXT3.cpp
)
endif()