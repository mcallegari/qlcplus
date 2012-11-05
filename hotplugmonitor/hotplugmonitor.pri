unix:!macx:CONFIG += udev
macx:CONFIG       += iokit
win32:CONFIG      += win32

CONFIG(udev) {
    CONFIG    += link_pkgconfig
    PKGCONFIG += libudev
}

CONFIG(iokit) {
    LIBS      += -framework IOKit -framework CoreFoundation
}
