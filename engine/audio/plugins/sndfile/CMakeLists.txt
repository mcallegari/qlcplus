include(../../../../variables.cmake)

add_library(sndfileplugin)
target_sources(sndfileplugin PRIVATE
    ../../src/audiodecoder.cpp ../../src/audiodecoder.h
    ../../src/audioparameters.cpp ../../src/audioparameters.h
    audiodecoder_sndfile.cpp audiodecoder_sndfile.h
)
target_include_directories(sndfileplugin PRIVATE
    ../../src
)

target_link_libraries(sndfileplugin PRIVATE
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
)

install(TARGETS sndfileplugin
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
