set(module_name "e131")

set(TS_FILES
    E131_de_DE.ts
    E131_es_ES.ts
    E131_fi_FI.ts
    E131_fr_FR.ts
    E131_it_IT.ts
    E131_nl_NL.ts
    E131_cz_CZ.ts
    E131_pt_BR.ts
    E131_ca_ES.ts
    E131_ja_JP.ts
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
    ../interfaces/qlcioplugin.cpp ../interfaces/qlcioplugin.h
    configuree131.cpp configuree131.h configuree131.ui
    e131controller.cpp e131controller.h
    e131packetizer.cpp e131packetizer.h
    e131plugin.cpp e131plugin.h
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
   install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/org.qlcplus.QLCPlus.e131.metainfo.xml"
           DESTINATION ${METAINFODIR})
endif()
