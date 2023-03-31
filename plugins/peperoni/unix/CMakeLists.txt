include(../../../variables.cmake)

add_library(peperoni)
target_sources(peperoni PRIVATE
    ../../interfaces/qlcioplugin.cpp ../../interfaces/qlcioplugin.h
    peperoni.cpp peperoni.h
    peperonidevice.cpp peperonidevice.h
)
target_include_directories(peperoni PRIVATE
    ../../interfaces
    ../common
)

target_link_libraries(peperoni PRIVATE
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::Widgets
)

# libusb-1.0
pkg_check_modules(LIBUSB_1 IMPORTED_TARGET libusb-1.0)

if(${LIBUSB_1_FOUND})
    include_directories(
        ${LIBUSB_1_INCLUDE_DIRS}
    )

    link_libraries(
        ${LIBUSB_1_LIBRARIES}
    )
endif()

install(TARGETS peperoni
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
