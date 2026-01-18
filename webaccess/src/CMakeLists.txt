set(module_name "qlcpluswebaccess")

include(../../variables.cmake)

set(TS_FILES
    webaccess_fi_FI.ts
    webaccess_de_DE.ts
    webaccess_fr_FR.ts
    webaccess_es_ES.ts
    webaccess_it_IT.ts
    webaccess_nl_NL.ts
    webaccess_cz_CZ.ts
    webaccess_pt_BR.ts
    webaccess_ca_ES.ts
    webaccess_ja_JP.ts
)

if(QT_VERSION_MAJOR GREATER 5)
    qt_add_translation(QM_FILES ${TS_FILES})
else()
    qt5_add_translation(QM_FILES ${TS_FILES})
endif()

set(WEBACCESS_SOURCES
    commonjscss.h
    qhttpserver/http_parser.c qhttpserver/http_parser.h
    qhttpserver/qhttpconnection.cpp qhttpserver/qhttpconnection.h
    qhttpserver/qhttprequest.cpp qhttpserver/qhttprequest.h
    qhttpserver/qhttpresponse.cpp qhttpserver/qhttpresponse.h
    qhttpserver/qhttpserver.cpp qhttpserver/qhttpserver.h
    qhttpserver/qhttpserverfwd.h
    webaccessbase.cpp webaccessbase.h
    webaccessauth.cpp webaccessauth.h
    webaccessconfiguration.cpp webaccessconfiguration.h
    webaccesssimpledesk.cpp webaccesssimpledesk.h
    ${QM_FILES}
)

if(qmlui)
    list(APPEND WEBACCESS_SOURCES
        webaccess-qml.cpp webaccess-qml.h
    )
else()
    list(APPEND WEBACCESS_SOURCES
        webaccess.cpp webaccess.h
    )
endif()

set(webaccess_lib_type SHARED)
if(WIN32 AND qmlui)
    set(webaccess_lib_type STATIC)
endif()

add_library(${module_name} ${webaccess_lib_type} ${WEBACCESS_SOURCES})

target_include_directories(${module_name} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ../../engine/audio/src
    ../../engine/src
    qhttpserver
)

if(qmlui)
    target_include_directories(${module_name} PUBLIC
        ../../qmlui
        ../../qmlui/virtualconsole
    )
else()
    target_include_directories(${module_name} PUBLIC
        ../../ui/src
        ../../ui/src/virtualconsole
    )
endif()

target_compile_definitions(${module_name} PUBLIC
    NO_SSL
    USE_WEBSOCKET
)

find_package(Qt${QT_MAJOR_VERSION} REQUIRED COMPONENTS WebSockets)

target_link_libraries(${module_name} PUBLIC
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::Multimedia
    Qt${QT_MAJOR_VERSION}::Network
    Qt${QT_MAJOR_VERSION}::Quick
    Qt${QT_MAJOR_VERSION}::Widgets
    Qt${QT_MAJOR_VERSION}::WebSockets
    qlcplusengine
)

if(qmlui OR (QT_VERSION_MAJOR GREATER 5))
    target_link_libraries(${module_name} PUBLIC
        Qt${QT_MAJOR_VERSION}::Qml
    )
endif()

if(NOT (qmlui OR (QT_VERSION_MAJOR GREATER 5)))
    target_link_libraries(${module_name} PUBLIC
        Qt${QT_MAJOR_VERSION}::Script
    )
endif()

if(NOT qmlui)
    target_link_libraries(${module_name} PUBLIC
        qlcplusui
    )
endif()

if(WIN32)
    target_include_directories(${module_name} PUBLIC /)
    target_link_libraries(${module_name} PUBLIC ws2_32)
endif()

if(UNIX AND NOT APPLE)
    target_sources(${module_name} PRIVATE
        webaccessnetwork.cpp webaccessnetwork.h
    )
endif()


install(TARGETS ${module_name}
    ARCHIVE DESTINATION ${INSTALLROOT}/${LIBSDIR}
    LIBRARY DESTINATION ${INSTALLROOT}/${LIBSDIR}
    RUNTIME DESTINATION ${INSTALLROOT}/${LIBSDIR}
)
