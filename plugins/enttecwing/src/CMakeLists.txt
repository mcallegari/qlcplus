set(module_name "enttecwing")

set(TS_FILES
    ENTTEC_Wing_fi_FI.ts
    ENTTEC_Wing_de_DE.ts
    ENTTEC_Wing_es_ES.ts
    ENTTEC_Wing_fr_FR.ts
    ENTTEC_Wing_it_IT.ts
    ENTTEC_Wing_nl_NL.ts
    ENTTEC_Wing_cz_CZ.ts
    ENTTEC_Wing_pt_BR.ts
    ENTTEC_Wing_ca_ES.ts
    ENTTEC_Wing_ja_JP.ts
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

target_sources(${module_name} PRIVATE
    ../../interfaces/qlcioplugin.cpp ../../interfaces/qlcioplugin.h
    ${module_name}.cpp ${module_name}.h
    playbackwing.cpp playbackwing.h
    programwing.cpp programwing.h
    shortcutwing.cpp shortcutwing.h
    wing.cpp wing.h
)
target_include_directories(${module_name} PRIVATE
    ../../interfaces
)

target_link_libraries(${module_name} PRIVATE
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::Network
    Qt${QT_MAJOR_VERSION}::Widgets
)

if(WIN32)
    target_compile_definitions(${module_name} PRIVATE
        QLC_EXPORT
    )
endif()

install(TARGETS ${module_name}
    LIBRARY DESTINATION ${INSTALLROOT}/${PLUGINDIR}
    RUNTIME DESTINATION ${INSTALLROOT}/${PLUGINDIR}
)

if (UNIX AND NOT APPLE)
   install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/org.qlcplus.QLCPlus.enttecwing.metainfo.xml"
           DESTINATION "${METAINFODIR}")
endif()
