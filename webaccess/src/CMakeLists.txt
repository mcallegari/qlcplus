qt_add_library(qlcpluswebaccess
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
    Qt::Core
    Qt::Gui
    Qt::Multimedia
    Qt::Network
    Qt::Widgets
    qlcplusengine
    qlcplusui
)

target_compile_options(qlcpluswebaccess
    -Werror
)

if(qmlui OR (QT_VERSION_MAJOR GREATER 5))
    target_link_libraries(qlcpluswebaccess PUBLIC
        Qt::Qml
    )
endif()

if(NOT (qmlui OR (QT_VERSION_MAJOR GREATER 5)))
    target_link_libraries(qlcpluswebaccess PUBLIC
        Qt::Script
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
    target_sources(qlcpluswebaccess PUBLIC
        webaccessnetwork.cpp webaccessnetwork.h
    )
endif()

if(MACOS)
    set_target_properties(qlcpluswebaccess PROPERTIES
        MACOSX_BUNDLE FALSE
    )
endif()

if(qmlui)
    target_compile_definitions(qlcpluswebaccess PUBLIC
        QMLUI
    )
endif()

if((NOT MACOS AND NOT IOS) AND (system(g++--version OR grep-e"4.6.[0-9]")))
    target_compile_options(qlcpluswebaccess
        -Wno-error=strict-overflow
    )
endif()

if((NOT MACOS AND NOT IOS) AND (NOT (system(g++--version OR grep-e"4.6.[0-9]"))))
    target_compile_options(qlcpluswebaccess
        -Wno-unused-local-typedefs
    )
endif()

if(CONFIG(coverage))
    target_link_libraries(qlcpluswebaccess PUBLIC
        gcov
    )

    target_compile_options(qlcpluswebaccess
        -fprofile-arcs
        -ftest-coverage
    )
endif()

install(TARGETS qlcpluswebaccess
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
