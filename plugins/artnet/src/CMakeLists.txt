set(module_name "artnet")

set(TS_FILES
    ArtNet_de_DE.ts
    ArtNet_es_ES.ts
    ArtNet_fi_FI.ts
    ArtNet_fr_FR.ts
    ArtNet_it_IT.ts
    ArtNet_nl_NL.ts
    ArtNet_cz_CZ.ts
    ArtNet_pt_BR.ts
    ArtNet_ca_ES.ts
    ArtNet_ja_JP.ts
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
    ../../interfaces/rdmprotocol.cpp ../../interfaces/rdmprotocol.h
    artnetcontroller.cpp artnetcontroller.h
    artnetpacketizer.cpp artnetpacketizer.h
    artnetplugin.cpp artnetplugin.h
    configureartnet.cpp configureartnet.h configureartnet.ui
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

install(TARGETS ${module_name}
    LIBRARY DESTINATION ${INSTALLROOT}/${PLUGINDIR}
    RUNTIME DESTINATION ${INSTALLROOT}/${PLUGINDIR}
)
