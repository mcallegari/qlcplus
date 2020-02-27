set(qlcpluswebaccess_src
    src/qhttpserver/http_parser.c 
    src/qhttpserver/qhttpconnection.cpp 
    src/qhttpserver/qhttprequest.cpp 
    src/qhttpserver/qhttpresponse.cpp 
    src/qhttpserver/qhttpserver.cpp
    
    src/webaccess.cpp
    src/webaccessconfiguration.cpp
    src/webaccesssimpledesk.cpp
    src/webaccessauth.cpp
    src/webaccessnetwork.cpp
)

include_directories(
    ${CMAKE_BINARY_DIR}/engine/
    ${CMAKE_SOURCE_DIR}/engine/src/
    ${CMAKE_SOURCE_DIR}/engine/audio/src/    
    ${CMAKE_BINARY_DIR}/ui/src/
    ${CMAKE_SOURCE_DIR}/ui/src/
    ${CMAKE_SOURCE_DIR}/ui/src/virtualconsole/
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qhttpserver/
)

add_library(qlcpluswebaccess SHARED ${qlcpluswebaccess_src})

target_link_libraries(qlcpluswebaccess
    Qt5::Core 
    Qt5::Widgets
    Qt5::Script 
    Qt5::Network
)
