set(module_name "os2l")

set(TS_FILES
    OS2L_de_DE.ts
    OS2L_es_ES.ts
    # OS2L_fi_FI.ts
    OS2L_fr_FR.ts
    OS2L_it_IT.ts
    OS2L_nl_NL.ts
    # OS2L_cz_CZ.ts
    # OS2L_pt_BR.ts
    OS2L_ca_ES.ts
    OS2L_ja_JP.ts
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
    os2lconfiguration.cpp os2lconfiguration.h os2lconfiguration.ui
    os2lplugin.cpp os2lplugin.h
)
target_include_directories(${module_name} PRIVATE
    ../interfaces
)

target_link_libraries(${module_name} PRIVATE
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::Network
    Qt${QT_MAJOR_VERSION}::Widgets
)

install(TARGETS ${module_name}
    LIBRARY DESTINATION ${INSTALLROOT}/${PLUGINDIR}
    RUNTIME DESTINATION ${INSTALLROOT}/${PLUGINDIR}
)

if (UNIX AND NOT APPLE)
   install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/org.qlcplus.QLCPlus.os2l.metainfo.xml"
           DESTINATION ${METAINFODIR})
endif()
