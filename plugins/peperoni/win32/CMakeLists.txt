qt_add_plugin(peperoni)
target_sources(peperoni PRIVATE
    ../../interfaces/qlcioplugin.cpp ../../interfaces/qlcioplugin.h
    peperoni.cpp peperoni.h
    peperoni/usbdmx-dynamic.cpp peperoni/usbdmx-dynamic.h
    peperonidevice.cpp peperonidevice.h
)
target_include_directories(peperoni PRIVATE
    ../../interfaces
    peperoni
)

target_link_libraries(peperoni PRIVATE
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::Widgets
)

install(TARGETS peperoni
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
