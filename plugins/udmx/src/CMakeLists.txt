include(../../../variables.cmake)

set(module_name "udmx")

set(TS_FILES
    uDMX_fi_FI.ts
    uDMX_de_DE.ts
    uDMX_es_ES.ts
    uDMX_fr_FR.ts
    uDMX_it_IT.ts
    uDMX_nl_NL.ts
    uDMX_cz_CZ.ts
    uDMX_pt_BR.ts
    uDMX_ca_ES.ts
    uDMX_ja_JP.ts
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
)

target_include_directories(${module_name} PRIVATE
    ../../interfaces
)

target_link_libraries(${module_name} PRIVATE
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::Widgets
)

if(WIN32)
    target_sources(${module_name} PRIVATE
        libusb_dyn.c libusb_dyn.h
    )
else()
    target_sources(${module_name} PRIVATE
        ${module_name}.cpp ${module_name}.h
        udmxdevice.cpp udmxdevice.h
    )
endif()

pkg_check_modules(LIBUSB_1 IMPORTED_TARGET libusb-1.0)

if(${LIBUSB_1_FOUND})
    include_directories(
        ${LIBUSB_1_INCLUDE_DIRS}
    )

    link_libraries(
        ${LIBUSB_1_LIBRARIES}
    )
endif()

install(TARGETS ${module_name}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
