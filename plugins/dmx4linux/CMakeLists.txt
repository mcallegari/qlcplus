add_library(dmx4linux)
target_sources(dmx4linux PRIVATE
    ../interfaces/qlcioplugin.cpp ../interfaces/qlcioplugin.h
    dmx4linux.cpp dmx4linux.h
)
target_include_directories(dmx4linux PRIVATE
    ../interfaces
)

target_link_libraries(dmx4linux PRIVATE
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
)

install(TARGETS dmx4linux
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
