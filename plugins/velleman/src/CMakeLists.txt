set(module_name "velleman")

set(TS_FILES
    Velleman_fi_FI.ts
    Velleman_de_DE.ts
    Velleman_es_ES.ts
    Velleman_fr_FR.ts
    Velleman_it_IT.ts
    Velleman_nl_NL.ts
    Velleman_cz_CZ.ts
    Velleman_pt_BR.ts
    Velleman_ca_ES.ts
    Velleman_ja_JP.ts
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

    set(K8062DDIR "C:/Qt/K8062D")
    target_link_directories(${module_name} PRIVATE
        ${K8062DDIR}
    )
    target_link_libraries(${module_name} PRIVATE
        K8062D
    )
    target_link_libraries(${module_name} PRIVATE
        ${K8062DDIR}/K8062D.a
    )
    target_include_directories(${module_name} PRIVATE
        ${K8062DDIR}
    )
else()
    target_sources(${module_name} PRIVATE
        velleman_mock.cpp
    )
endif()

# Installation only on Windows; Unix targets are built only for unit testing.
if(WIN32)
    install(TARGETS ${module_name}
        LIBRARY DESTINATION ${INSTALLROOT}/${PLUGINDIR}
        RUNTIME DESTINATION ${INSTALLROOT}/${PLUGINDIR}
    )
endif()
