add_library(os2l)
target_sources(os2l PRIVATE
    ../interfaces/qlcioplugin.cpp ../interfaces/qlcioplugin.h
    os2lconfiguration.cpp os2lconfiguration.h os2lconfiguration.ui
    os2lplugin.cpp os2lplugin.h
)
target_include_directories(os2l PRIVATE
    ../interfaces
)

target_link_libraries(os2l PRIVATE
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::Network
    Qt${QT_MAJOR_VERSION}::Widgets
)

install(TARGETS os2l
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
