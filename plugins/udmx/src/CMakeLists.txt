find_package(PkgConfig REQUIRED)

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

add_library(${module_name} SHARED ${QM_FILES})

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

target_sources(${module_name} PRIVATE
	${module_name}.cpp ${module_name}.h
	udmxdevice.cpp udmxdevice.h
)

pkg_check_modules(LIBUSB1 IMPORTED_TARGET libusb-1.0)

if(${LIBUSB1_FOUND})
    target_include_directories(${module_name} PRIVATE ${LIBUSB_1_INCLUDE_DIRS})
    target_link_libraries(${module_name} PRIVATE ${LIBUSB_1_LINK_LIBRARIES})
endif()

install(TARGETS ${module_name}
    LIBRARY DESTINATION ${INSTALLROOT}/${PLUGINDIR}
    RUNTIME DESTINATION ${INSTALLROOT}/${PLUGINDIR}
)

if (UNIX AND NOT APPLE)
   install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/z65-anyma-udmx.rules"
           DESTINATION ${UDEVRULESDIR})
   install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/org.qlcplus.QLCPlus.udmx.metainfo.xml"
           DESTINATION ${METAINFODIR})
endif()
