include(../../../variables.cmake)

add_library(dmxusb)

target_sources(dmxusb PRIVATE
    ../../interfaces/qlcioplugin.cpp ../../interfaces/qlcioplugin.h
    ../../interfaces/rdmprotocol.cpp ../../interfaces/rdmprotocol.h
    ../../midi/src/common/midiprotocol.cpp ../../midi/src/common/midiprotocol.h
    dmxinterface.cpp dmxinterface.h
    dmxusb.cpp dmxusb.h
    dmxusbconfig.cpp dmxusbconfig.h
    dmxusbopenrx.cpp dmxusbopenrx.h
    dmxusbwidget.cpp dmxusbwidget.h
    enttecdmxusbopen.cpp enttecdmxusbopen.h
    enttecdmxusbpro.cpp enttecdmxusbpro.h
    stageprofi.cpp stageprofi.h
    vinceusbdmx512.cpp vinceusbdmx512.h
)

target_include_directories(dmxusb PRIVATE
    ../../interfaces
    ../../midi/src/common
)

target_link_libraries(dmxusb PRIVATE
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::Widgets
)

if(WIN32)
    target_compile_definitions(dmxusb PRIVATE FTD2XX)
    message("Building with FTD2xx support.")
endif()

if(UNIX)
    target_sources(dmxusb PUBLIC
        euroliteusbdmxpro.cpp euroliteusbdmxpro.h
        nanodmx.cpp nanodmx.h
    )
endif()

if(MACOS)
    set_target_properties(dmxusb PROPERTIES
        MACOSX_BUNDLE FALSE
    )
endif()

if(qtserial)
    target_sources(dmxusb PUBLIC
        qtserial-interface.cpp qtserial-interface.h
    )

    target_compile_definitions(dmxusb PRIVATE
        QTSERIAL
    )

    target_link_libraries(dmxusb PRIVATE
        Qt${QT_MAJOR_VERSION}::SerialPort
    )
endif()

if(ftd2xx)
    target_sources(dmxusb PUBLIC
        ftd2xx-interface.cpp ftd2xx-interface.h
    )

    target_compile_definitions(dmxusb PRIVATE
        FTD2XX
    )
endif()

if((ftd2xx) AND (WIN32))
    target_include_directories(dmxusb PRIVATE
        C:/Qt/D2XXSDK
    )

    target_link_libraries(dmxusb PRIVATE
        # Remove: LC:/Qt/D2XXSDK/i386
        C:/Qt/D2XXSDK/i386/libftd2xx.a
        ftd2xx
    )
endif()

if(libftdi)
    target_sources(dmxusb PUBLIC
        libftdi-interface.cpp libftdi-interface.h
    )
endif()

install(TARGETS dmxusb
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
