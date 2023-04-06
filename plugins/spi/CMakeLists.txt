include(../../variables.cmake)

set(module_name "spi")

set(TS_FILES
    SPI_de_DE.ts
    SPI_es_ES.ts
    SPI_fi_FI.ts
    SPI_fr_FR.ts
    SPI_it_IT.ts
    SPI_nl_NL.ts
    SPI_cz_CZ.ts
    SPI_pt_BR.ts
    SPI_ca_ES.ts
    SPI_ja_JP.ts
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
    ../interfaces/qlcioplugin.cpp ../interfaces/qlcioplugin.h
    spiconfiguration.cpp spiconfiguration.h spiconfiguration.ui
    spioutthread.cpp spioutthread.h
    spiplugin.cpp spiplugin.h
)
target_include_directories(${module_name} PRIVATE
    $SYSROOT/usr/include
    ../interfaces
)

target_link_libraries(${module_name} PRIVATE
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::Widgets
)

install(TARGETS ${module_name}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES ${QM_FILES} DESTINATION ${INSTALLROOT}/${TRANSLATIONDIR})
