add_library(udmx)
target_sources(udmx PRIVATE
    ../../interfaces/qlcioplugin.cpp ../../interfaces/qlcioplugin.h
    udmx.cpp udmx.h
    udmxdevice.cpp udmxdevice.h
)
target_include_directories(udmx PRIVATE
    ../../interfaces
)

target_link_libraries(udmx PRIVATE
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::Widgets
)

if(WIN32)
    target_sources(udmx PRIVATE
        libusb_dyn.c libusb_dyn.h
    )
endif()

install(TARGETS udmx
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
