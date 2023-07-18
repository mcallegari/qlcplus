set(module_name "loopback")

set(TS_FILES
    loopback_fi_FI.ts
    loopback_de_DE.ts
    loopback_es_ES.ts
    loopback_fr_FR.ts
    loopback_it_IT.ts
    loopback_nl_NL.ts
    loopback_cz_CZ.ts
    loopback_pt_BR.ts
    loopback_ca_ES.ts
    loopback_ja_JP.ts
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
)
target_include_directories(${module_name} PRIVATE
    ../../interfaces
)

target_link_libraries(${module_name} PRIVATE
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
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
   install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/org.qlcplus.QLCPlus.loopback.metainfo.xml"
           DESTINATION ${METAINFODIR})
endif()
