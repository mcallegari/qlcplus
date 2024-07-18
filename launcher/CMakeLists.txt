set(module_name "qlcplus-launcher")

set(TS_FILES
    launcher_fi_FI.ts
    launcher_de_DE.ts
    launcher_fr_FR.ts
    launcher_es_ES.ts
    launcher_it_IT.ts
    launcher_nl_NL.ts
    launcher_cz_CZ.ts
    launcher_pt_BR.ts
    launcher_ca_ES.ts
    launcher_ja_JP.ts
)

if(QT_VERSION_MAJOR GREATER 5)
    qt_add_translation(QM_FILES ${TS_FILES})
else()
    qt5_add_translation(QM_FILES ${TS_FILES})
endif()

add_executable(${module_name}
    launcher.cpp launcher.h
    main.cpp
    ${QM_FILES}
)

target_include_directories(${module_name} PRIVATE
    ../engine/src
    ../plugins/interfaces
)

target_link_libraries(${module_name} PRIVATE
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::Widgets
)

# Resources:
set_source_files_properties("../resources/icons/png/qlcplus-fixtureeditor.png"
    PROPERTIES QT_RESOURCE_ALIAS "qlcplus-fixtureeditor.png"
)
set_source_files_properties("../resources/icons/png/qlcplus.png"
    PROPERTIES QT_RESOURCE_ALIAS "qlcplus.png"
)
set(launcher_resource_files
    "../resources/icons/png/qlcplus-fixtureeditor.png"
    "../resources/icons/png/qlcplus.png"
)

if(QT_VERSION_MAJOR GREATER 5)
    qt_add_resources(${module_name} "launcher"
    PREFIX
        "/"
    BASE
        "."
    FILES
        ${launcher_resource_files}
)
else()
    qt5_add_resources($launcher.qrc)
    target_sources(${module_name} PRIVATE
        ${launcher_resource_files})
endif()

install(TARGETS ${module_name}
    DESTINATION ${INSTALLROOT}/${BINDIR}
)
