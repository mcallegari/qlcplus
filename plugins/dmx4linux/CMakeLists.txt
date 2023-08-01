set(module_name "dmx4linux")

set(TS_FILES
    DMX4Linux_de_DE.ts
    DMX4Linux_es_ES.ts
    DMX4Linux_fi_FI.ts
    DMX4Linux_fr_FR.ts
    DMX4Linux_it_IT.ts
    DMX4Linux_nl_NL.ts
    DMX4Linux_cz_CZ.ts
    DMX4Linux_pt_BR.ts
    DMX4Linux_ca_ES.ts
    DMX4Linux_ja_JP.ts
)

if(QT_VERSION_MAJOR GREATER 5)
    qt_add_translation(QM_FILES ${TS_FILES})
else()
    qt5_add_translation(QM_FILES ${TS_FILES})
endif()

add_library(${module_name} SHARED
    ${QM_FILES}
)

target_sources(${module_name} PRIVATE
    ../interfaces/qlcioplugin.cpp ../interfaces/qlcioplugin.h
    ${module_name}.cpp ${module_name}.h
)
target_include_directories(${module_name} PRIVATE
    ../interfaces
)

target_link_libraries(${module_name} PRIVATE
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
)

install(TARGETS ${module_name}
    LIBRARY DESTINATION ${INSTALLROOT}/${PLUGINDIR}
    RUNTIME DESTINATION ${INSTALLROOT}/${PLUGINDIR}
)

install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/org.qlcplus.QLCPlus.dmx4linux.metainfo.xml" DESTINATION ${METAINFODIR})
