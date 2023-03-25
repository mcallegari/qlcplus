qt_add_plugin(midiplugin)
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
    Qt::Core
    Qt::Gui
    Qt::Widgets
    winmm
)

target_compile_options(midiplugin
    -Werror
)

if(qmlui)
    target_compile_definitions(midiplugin PRIVATE
        QMLUI
    )
endif()

if((NOT MACOS AND NOT IOS) AND (system(g++--version OR grep-e"4.6.[0-9]")))
    target_compile_options(midiplugin
        -Wno-error=strict-overflow
    )
endif()

if((NOT MACOS AND NOT IOS) AND (NOT (system(g++--version OR grep-e"4.6.[0-9]"))))
    target_compile_options(midiplugin
        -Wno-unused-local-typedefs
    )
endif()

if(MACOS)
    set_target_properties(midiplugin PROPERTIES
        MACOSX_BUNDLE FALSE
    )
endif()

install(TARGETS midiplugin
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
