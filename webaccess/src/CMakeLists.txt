include(../../variables.cmake)
include(../../coverage.cmake)

add_library(qlcpluswebaccess
    commonjscss.h
    qhttpserver/http_parser.c qhttpserver/http_parser.h
    qhttpserver/qhttpconnection.cpp qhttpserver/qhttpconnection.h
    qhttpserver/qhttprequest.cpp qhttpserver/qhttprequest.h
    qhttpserver/qhttpresponse.cpp qhttpserver/qhttpresponse.h
    qhttpserver/qhttpserver.cpp qhttpserver/qhttpserver.h
    qhttpserver/qhttpserverfwd.h
    webaccess.cpp webaccess.h
    webaccessauth.cpp webaccessauth.h
    webaccessconfiguration.cpp webaccessconfiguration.h
    webaccesssimpledesk.cpp webaccesssimpledesk.h
)
target_include_directories(qlcpluswebaccess PUBLIC
    ../../engine/audio/src
    ../../engine/src
    ../../ui/src
    ../../ui/src/virtualconsole
    qhttpserver
)

target_compile_definitions(qlcpluswebaccess PUBLIC
    NO_SSL
    USE_WEBSOCKET
)

target_link_libraries(qlcpluswebaccess PUBLIC
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::Multimedia
    Qt${QT_MAJOR_VERSION}::Network
    Qt${QT_MAJOR_VERSION}::Widgets
    qlcplusengine
    qlcplusui
)

if(qmlui OR (QT_VERSION_MAJOR GREATER 5))
    target_link_libraries(qlcpluswebaccess PUBLIC
        Qt${QT_MAJOR_VERSION}::Qml
    )
endif()

if(NOT (qmlui OR (QT_VERSION_MAJOR GREATER 5)))
    target_link_libraries(qlcpluswebaccess PUBLIC
        Qt${QT_MAJOR_VERSION}::Script
    )
endif()

if(WIN32)
    target_include_directories(qlcpluswebaccess PUBLIC
        /
    )

    target_link_libraries(qlcpluswebaccess PUBLIC
        ws2_32
    )
endif()

if(UNIX AND NOT MACOS)
    target_sources(qlcpluswebaccess PRIVATE
        webaccessnetwork.cpp webaccessnetwork.h
    )
endif()


install(TARGETS qlcpluswebaccess
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
