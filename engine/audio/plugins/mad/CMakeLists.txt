include(../../../../variables.cmake)

add_library(madplugin)
target_sources(madplugin PRIVATE
    ../../src/audiodecoder.cpp ../../src/audiodecoder.h
    ../../src/audioparameters.cpp ../../src/audioparameters.h
    audiodecoder_mad.cpp audiodecoder_mad.h
)
target_include_directories(madplugin PRIVATE
    ../../src
)

target_link_libraries(madplugin PRIVATE
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
)

install(TARGETS madplugin
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
