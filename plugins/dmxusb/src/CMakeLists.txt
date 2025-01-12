find_package(PkgConfig REQUIRED)  # Make sure pkg-config is available

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
    SHARED
    ${QM_FILES}
)

if(WIN32)
    set(FTD2XXDIR "C:/projects/D2XXSDK")
    if ($ENV{MSYSTEM_CARCH} STREQUAL "i686")
        target_link_libraries(${module_name} PRIVATE ${FTD2XXDIR}/i386/libftd2xx.a)
    else()
        target_link_libraries(${module_name} PRIVATE ${FTD2XXDIR}/amd64/libftd2xx.a)
    endif()
    target_include_directories(${module_name} PRIVATE ${FTD2XXDIR})
	message("Building with FTD2xx support.")
    set(WITH_D2XX TRUE)
endif()

if(UNIX)
    pkg_check_modules(LIBFTDI1 IMPORTED_TARGET libftdi1 libusb-1.0)
    pkg_check_modules(LIBFTDI IMPORTED_TARGET libftdi libusb)

    if (LIBFTDI1_FOUND)
        # Add the include directories to the project
        target_include_directories(${module_name} PRIVATE ${LIBFTDI1_INCLUDE_DIRS})

        target_compile_definitions(${module_name} PRIVATE LIBFTDI1)
        execute_process(COMMAND pkg-config --modversion libftdi1 OUTPUT_VARIABLE LIBFTDI1_VERSION)  # Get libFTDI1 version

        # Link your executable against the libftdi1 library
        target_link_libraries(${module_name} PRIVATE ${LIBFTDI1_LINK_LIBRARIES})

        if (APPLE)
            #include(../../../platforms/macos/nametool.pri)
            #add_custom_command(TARGET ${module_name}
            #    COMMAND ${PKG_CONFIG_NAMETOOL} libusb-1.0 libusb-1.0.0.dylib
            #    COMMAND ${PKG_CONFIG_NAMETOOL} libftdi1 libftdi1.2.dylib
            #    COMMENT "Creating symlinks for libusb-1.0.0.dylib and libftdi1.2.dylib"
            #)
        endif()

        message("Building with libFTDI1 support. Version: ${LIBFTDI1_VERSION}")
        set(WITH_LIBFTDI TRUE)

        if (NOT ${LIBFTDI1_VERSION} VERSION_LESS "1.5")
            message("Using v1.5+ buffer flush API")
            target_compile_definitions(${module_name} PRIVATE LIBFTDI1_5)
        endif()

    elseif (LIBFTDI_FOUND)
        # Add the include directories to the project
        target_include_directories(${module_name} PRIVATE ${LIBFTDI_INCLUDE_DIRS})

        # Link your executable against the libftdi library
        link_libraries(LIBFTDI_LINK_LIBRARIES)

        target_compile_definitions(${module_name} PRIVATE LIBFTDI)
        target_link_libraries(${module_name} PRIVATE ${LIBFTDI_LIBRARIES})

        if (APPLE)
            #include(../../../platforms/macos/nametool.pri)
            #add_custom_command(TARGET ${module_name}
            #    COMMAND ${PKG_CONFIG_NAMETOOL} libftdi libftdi.1.dylib
            #    COMMENT "Creating symlink for libftdi.1.dylib"
            #)
        endif()

        message("Building with libFTDI support.")
        set(WITH_LIBFTDI TRUE)

    else()
        message(FATAL_ERROR "Neither libftdi-0.X nor libftdi-1.X found!")
    endif()

endif()

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

if(UNIX)
    target_sources(${module_name} PUBLIC
        euroliteusbdmxpro.cpp euroliteusbdmxpro.h
        nanodmx.cpp nanodmx.h
    )
endif()

if(APPLE)
    set_target_properties(${module_name} PROPERTIES
        MACOSX_BUNDLE FALSE
    )
endif()

# always include QtSerialPort
target_sources(${module_name} PUBLIC
    qtserial-interface.cpp qtserial-interface.h
)

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS SerialPort)

target_compile_definitions(${module_name} PRIVATE
    QTSERIAL
)

target_link_libraries(${module_name} PRIVATE
    Qt${QT_MAJOR_VERSION}::SerialPort
)

if(WITH_D2XX)
    target_sources(${module_name} PUBLIC ftd2xx-interface.cpp ftd2xx-interface.h)
    target_compile_definitions(${module_name} PRIVATE FTD2XX)
endif()

if(WITH_LIBFTDI)
    target_sources(${module_name} PUBLIC libftdi-interface.cpp libftdi-interface.h)
endif()

if (UNIX AND NOT APPLE)
   # Rules to make USB DMX devices readable & writable by normal users
   install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/z65-dmxusb.rules"
           DESTINATION ${UDEVRULESDIR})
   install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/org.qlcplus.QLCPlus.dmxusb.metainfo.xml"
           DESTINATION ${METAINFODIR})
elseif (WITH_D2XX)
    if ($ENV{MSYSTEM_CARCH} STREQUAL "x86_64")
        install(FILES ${FTD2XXDIR}/amd64/ftd2xx64.dll DESTINATION  ${INSTALLROOT}/${LIBSDIR})
    endif()
endif()

install(TARGETS ${module_name}
    LIBRARY DESTINATION ${INSTALLROOT}/${PLUGINDIR}
    RUNTIME DESTINATION ${INSTALLROOT}/${PLUGINDIR}
)