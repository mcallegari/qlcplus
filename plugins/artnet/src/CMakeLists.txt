qt_add_plugin(artnet)
target_sources(artnet PRIVATE
    ../../interfaces/qlcioplugin.cpp ../../interfaces/qlcioplugin.h
    ../../interfaces/rdmprotocol.cpp ../../interfaces/rdmprotocol.h
    artnetcontroller.cpp artnetcontroller.h
    artnetpacketizer.cpp artnetpacketizer.h
    artnetplugin.cpp artnetplugin.h
    configureartnet.cpp configureartnet.h configureartnet.ui
)
target_include_directories(artnet PRIVATE
    ../../interfaces
)

target_link_libraries(artnet PRIVATE
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::Network
    Qt${QT_MAJOR_VERSION}::Widgets
)

install(TARGETS artnet
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
