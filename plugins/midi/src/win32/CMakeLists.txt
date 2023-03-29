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
    win32midienumerator.cpp
    win32midienumeratorprivate.h
    win32midiinputdevice.cpp win32midiinputdevice.h
    win32midioutputdevice.cpp win32midioutputdevice.h
)
target_include_directories(midiplugin PRIVATE
    ../../../../engine/src
    ../../../interfaces
    ../common ../common
)

target_link_libraries(midiplugin PRIVATE
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::Widgets
    winmm
)

install(TARGETS midiplugin
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
