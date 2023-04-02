include(../../../variables.cmake)

set(module_name "dmxusb")

set(TS_FILES
    DMX_USB_de_DE.ts
    DMX_USB_es_ES.ts
    DMX_USB_fi_FI.ts
    DMX_USB_fr_FR.ts
    DMX_USB_it_IT.ts
    DMX_USB_nl_NL.ts
    DMX_USB_cz_CZ.ts
    DMX_USB_pt_BR.ts
    DMX_USB_ca_ES.ts
    DMX_USB_ja_JP.ts
)

if(QT_VERSION_MAJOR GREATER 5)
    qt_add_translation(QM_FILES ${TS_FILES})
else()
    qt5_add_translation(QM_FILES ${TS_FILES})
endif()

add_library(${module_name}
    ${QM_FILES}
)

target_sources(${module_name} PRIVATE
    ../../interfaces/qlcioplugin.cpp ../../interfaces/qlcioplugin.h
    ../../interfaces/rdmprotocol.cpp ../../interfaces/rdmprotocol.h
    ../../midi/src/common/midiprotocol.cpp ../../midi/src/common/midiprotocol.h
    dmxinterface.cpp dmxinterface.h
    ${module_name}.cpp ${module_name}.h
    dmxusbconfig.cpp dmxusbconfig.h
    dmxusbopenrx.cpp dmxusbopenrx.h
    dmxusbwidget.cpp dmxusbwidget.h
    enttecdmxusbopen.cpp enttecdmxusbopen.h
    enttecdmxusbpro.cpp enttecdmxusbpro.h
    stageprofi.cpp stageprofi.h
    vinceusbdmx512.cpp vinceusbdmx512.h
)

target_include_directories(${module_name} PRIVATE
    ../../interfaces
    ../../midi/src/common
)

target_link_libraries(${module_name} PRIVATE
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::Widgets
)

if(WIN32)
    target_compile_definitions(${module_name} PRIVATE FTD2XX)
    message("Building with FTD2xx support.")
endif()

if(UNIX)
    target_sources(${module_name} PUBLIC
        euroliteusbdmxpro.cpp euroliteusbdmxpro.h
        nanodmx.cpp nanodmx.h
    )
endif()

if(MACOS)
    set_target_properties(${module_name} PROPERTIES
        MACOSX_BUNDLE FALSE
    )
endif()

if(qtserial)
    target_sources(${module_name} PUBLIC
        qtserial-interface.cpp qtserial-interface.h
    )

    target_compile_definitions(${module_name} PRIVATE
        QTSERIAL
    )

    target_link_libraries(${module_name} PRIVATE
        Qt${QT_MAJOR_VERSION}::SerialPort
    )
endif()

if(ftd2xx)
    target_sources(${module_name} PUBLIC
        ftd2xx-interface.cpp ftd2xx-interface.h
    )

    target_compile_definitions(${module_name} PRIVATE
        FTD2XX
    )
endif()

if((ftd2xx) AND (WIN32))
    target_include_directories(${module_name} PRIVATE
        C:/Qt/D2XXSDK
    )

    target_link_libraries(${module_name} PRIVATE
        # Remove: LC:/Qt/D2XXSDK/i386
        C:/Qt/D2XXSDK/i386/libftd2xx.a
        ftd2xx
    )
endif()

if(libftdi)
    target_sources(${module_name} PUBLIC
        libftdi-interface.cpp libftdi-interface.h
    )
endif()

install(TARGETS ${module_name}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
