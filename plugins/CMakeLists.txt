project(plugins)

# libusb-1.0
pkg_check_modules(LIBUSB_1 IMPORTED_TARGET libusb-1.0)

add_subdirectory(artnet)
add_subdirectory(E1.31)
add_subdirectory(loopback)
add_subdirectory(osc)
if(NOT ANDROID AND NOT IOS)
    add_subdirectory(dmxusb)
    add_subdirectory(peperoni)
    add_subdirectory(udmx)
    add_subdirectory(midi)
    add_subdirectory(velleman)
    add_subdirectory(enttecwing)
    add_subdirectory(hid)
    add_subdirectory(os2l)
endif()

pkg_check_modules(LIBOLA IMPORTED_TARGET libola)
pkg_check_modules(LIBOLASERVER IMPORTED_TARGET libolaserver)

if((((NOT ANDROID AND NOT IOS) AND (UNIX) AND (${LIBOLA_FOUND}) AND (${LIBOLASERVER_FOUND}))))
    add_subdirectory(ola)
endif()
if(UNIX AND NOT ANDROID AND NOT IOS AND NOT MACOS)
    add_subdirectory(dmx4linux)
    add_subdirectory(spi)
    # add_subdirectory(uart)
endif()

