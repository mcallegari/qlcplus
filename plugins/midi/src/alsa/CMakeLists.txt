include(../../../../variables.cmake)

add_library(midiplugin)
target_sources(midiplugin PRIVATE
    ../../../../engine/src/qlcfile.cpp ../../../../engine/src/qlcfile.h
    ../../../interfaces/qlcioplugin.cpp ../../../interfaces/qlcioplugin.h
    ../common/configuremidiplugin.cpp ../common/configuremidiplugin.h ../common/configuremidiplugin.ui
    ../common/mididevice.cpp ../common/mididevice.h
    ../common/midienumerator.h
    ../common/midiinputdevice.cpp ../common/midiinputdevice.h
    ../common/midioutputdevice.cpp ../common/midioutputdevice.h
    ../common/midiplugin.cpp ../common/midiplugin.h
    ../common/midiprotocol.cpp ../common/midiprotocol.h
    ../common/miditemplate.cpp ../common/miditemplate.h
    alsamidienumerator.cpp alsamidienumerator.h
    alsamidiinputdevice.cpp alsamidiinputdevice.h
    alsamidiinputthread.cpp alsamidiinputthread.h
    alsamidioutputdevice.cpp alsamidioutputdevice.h
    alsamidiutil.cpp alsamidiutil.h
)
target_include_directories(midiplugin PRIVATE
    ../../../../engine/src
    ../../../interfaces
    ../common
)

target_link_libraries(midiplugin PRIVATE
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::Widgets
)

install(TARGETS midiplugin
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
