include(../../variables.cmake)

set(module_name "osc")

set(TS_FILES
    OSC_de_DE.ts
    OSC_es_ES.ts
    OSC_fi_FI.ts
    OSC_fr_FR.ts
    OSC_it_IT.ts
    OSC_nl_NL.ts
    OSC_cz_CZ.ts
    OSC_pt_BR.ts
    OSC_ca_ES.ts
    OSC_ja_JP.ts
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
    configureosc.cpp configureosc.h configureosc.ui
    osccontroller.cpp osccontroller.h
    oscpacketizer.cpp oscpacketizer.h
    oscplugin.cpp oscplugin.h
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
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES ${QM_FILES} DESTINATION ${INSTALLROOT}/${TRANSLATIONDIR})
