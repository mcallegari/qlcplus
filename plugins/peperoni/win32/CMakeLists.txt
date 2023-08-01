set(module_name "peperoni")

set(TS_FILES
    ../Peperoni_fi_FI.ts
    ../Peperoni_de_DE.ts
    ../Peperoni_es_ES.ts
    ../Peperoni_fr_FR.ts
    ../Peperoni_it_IT.ts
    ../Peperoni_nl_NL.ts
    ../Peperoni_cz_CZ.ts
    ../Peperoni_pt_BR.ts
    ../Peperoni_ca_ES.ts
    ../Peperoni_ja_JP.ts
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
    ${module_name}/usbdmx-dynamic.cpp ${module_name}/usbdmx-dynamic.h
    peperonidevice.cpp peperonidevice.h
)
target_include_directories(${module_name} PRIVATE
    ../../interfaces
    ${module_name}
)

target_link_libraries(${module_name} PRIVATE
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::Widgets
)

install(TARGETS ${module_name}
    LIBRARY DESTINATION ${INSTALLROOT}/${PLUGINDIR}
    RUNTIME DESTINATION ${INSTALLROOT}/${PLUGINDIR}
)
