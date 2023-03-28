add_library(osc)
target_sources(osc PRIVATE
    ../interfaces/qlcioplugin.cpp ../interfaces/qlcioplugin.h
    configureosc.cpp configureosc.h configureosc.ui
    osccontroller.cpp osccontroller.h
    oscpacketizer.cpp oscpacketizer.h
    oscplugin.cpp oscplugin.h
)
target_include_directories(osc PRIVATE
    ../interfaces
)

target_link_libraries(osc PRIVATE
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::Network
    Qt${QT_MAJOR_VERSION}::Widgets
)

install(TARGETS osc
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
