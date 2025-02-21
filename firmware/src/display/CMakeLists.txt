if(CROSS_COMPILED)
# Build for testing on host.
target_sources(geeps_test PRIVATE

)
else()
target_include_directories(geeps PRIVATE
    arduino
    pervasive
)
# Build for embedded target
file(GLOB PERVASIVE_SOURCES "pervasive/*.cpp")
file(GLOB ARDUINO_SOURCES "arduino/*.cpp")
file(GLOB ARDUINO_API_SOURCES, "arduino/api/*.cpp")
target_sources(geeps PRIVATE
    epaper.cc
    ${ARDUINO_SOURCES}
    ${ARDUINO_API_SOURCES}
    ${PERVASIVE_SOURCES}
    # pervasive/hV_Colours565.cpp
    # pervasive/hV_Font_Terminal.cpp
    # pervasive/hV_Screen_Buffer.cpp
    # pervasive/hV_Utilities_Common.cpp
    # pervasive/Screen_EPD_EXT3.cpp
)
endif()