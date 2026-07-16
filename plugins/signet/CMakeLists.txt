set(module_name "signet")

set(TS_FILES
    SigNet_de_DE.ts
    SigNet_es_ES.ts
    SigNet_fi_FI.ts
    SigNet_fr_FR.ts
    SigNet_it_IT.ts
    SigNet_nl_NL.ts
    SigNet_cz_CZ.ts
    SigNet_pt_BR.ts
    SigNet_ca_ES.ts
    SigNet_ja_JP.ts
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
    ../interfaces/rdmprotocol.cpp ../interfaces/rdmprotocol.h
    configuresignet.cpp configuresignet.h configuresignet.ui
    signetcontroller.cpp signetcontroller.h
    signetpacketizer.cpp signetpacketizer.h
    signetplugin.cpp signetplugin.h
    sdk/sig-net-coap.cpp sdk/sig-net-coap.hpp
    sdk/sig-net-constants.hpp
    sdk/sig-net-crypto-qt.cpp sdk/sig-net-crypto.hpp
    sdk/sig-net-parse.cpp sdk/sig-net-parse.hpp
    sdk/sig-net-security.cpp sdk/sig-net-security.hpp
    sdk/sig-net-tlv.cpp sdk/sig-net-tlv.hpp
    sdk/sig-net-types.hpp
)

target_include_directories(${module_name} PRIVATE
    ../interfaces
    sdk
)

target_link_libraries(${module_name} PRIVATE
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::Network
    Qt${QT_MAJOR_VERSION}::Widgets
)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${module_name} PRIVATE -Wno-unused-variable)
endif()

install(TARGETS ${module_name}
    LIBRARY DESTINATION ${INSTALLROOT}/${PLUGINDIR}
    RUNTIME DESTINATION ${INSTALLROOT}/${PLUGINDIR}
)

if (UNIX AND NOT APPLE)
   install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/org.qlcplus.QLCPlus.signet.metainfo.xml"
           DESTINATION ${METAINFODIR})
endif()
