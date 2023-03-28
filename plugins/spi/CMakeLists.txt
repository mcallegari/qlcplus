qt_add_plugin(spi)
target_sources(spi PRIVATE
    ../interfaces/qlcioplugin.cpp ../interfaces/qlcioplugin.h
    spiconfiguration.cpp spiconfiguration.h spiconfiguration.ui
    spioutthread.cpp spioutthread.h
    spiplugin.cpp spiplugin.h
)
target_include_directories(spi PRIVATE
    $SYSROOT/usr/include
    ../interfaces
)

target_link_libraries(spi PRIVATE
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::Widgets
)

install(TARGETS spi
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
