qt_add_plugin(e131)
target_sources(e131 PRIVATE
    ../interfaces/qlcioplugin.cpp ../interfaces/qlcioplugin.h
    configuree131.cpp configuree131.h configuree131.ui
    e131controller.cpp e131controller.h
    e131packetizer.cpp e131packetizer.h
    e131plugin.cpp e131plugin.h
)
target_include_directories(e131 PRIVATE
    ../interfaces
)

target_link_libraries(e131 PRIVATE
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::Network
    Qt${QT_MAJOR_VERSION}::Widgets
)

install(TARGETS e131
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
