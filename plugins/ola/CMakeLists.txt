include(../../variables.cmake)

set(module_name "olaio")

set(TS_FILES
    OLA_fi_FI.ts
    OLA_de_DE.ts
    OLA_es_ES.ts
    OLA_fr_FR.ts
    OLA_it_IT.ts
    OLA_nl_NL.ts
    OLA_cz_CZ.ts
    OLA_pt_BR.ts
    OLA_ca_ES.ts
    OLA_ja_JP.ts
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
    configureolaio.cpp configureolaio.h configureolaio.ui
    ${module_name}.cpp ${module_name}.h
    olaoutthread.cpp olaoutthread.h
    qlclogdestination.cpp qlclogdestination.h
)
target_include_directories(${module_name} PRIVATE
    ../interfaces
)

target_link_libraries(${module_name} PRIVATE
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::Widgets
)

if(MACOS)
    set_target_properties(${module_name} PROPERTIES
        MACOSX_BUNDLE FALSE
    )
endif()

if((MACOS) AND (NOT (packagesExist(libolalibolaserver))))
    target_include_directories(${module_name} PRIVATE
        /opt/local/include
    )

    target_link_libraries(${module_name} PRIVATE
        # Remove: L/opt/local/lib
        ola
        olacommon
        olaserver
    )
endif()

if(NOT MACOS)
    target_link_libraries(${module_name} PRIVATE
        # Remove: L/usr/local/lib
        ola
        olacommon
        olaserver
    )
endif()

install(TARGETS ${module_name}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES ${QM_FILES} DESTINATION ${INSTALLROOT}/${TRANSLATIONDIR})
