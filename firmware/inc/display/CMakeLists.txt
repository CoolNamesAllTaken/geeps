if(CROSS_COMPILED)
# Build for testing on host.
target_sources(geeps_test PRIVATE

)
else()
# Build for embedded target
target_sources(geeps PRIVATE
    epaper.hh
    hV_Colours565.h
    hV_Configuration.h
    hV_Documentation.h
    hV_Font_Terminal.h
    hV_Font.h
    hV_Screen_Buffer.h
    hV_Utilities.h
    PDLS_EXT3_Basic.h
    Screen_EPD_EXT3.h
    Terminal6x8e.h
    Terminal8x12e.h
    Terminal12x16e.h
    Terminal16x24e.h
)
endif()