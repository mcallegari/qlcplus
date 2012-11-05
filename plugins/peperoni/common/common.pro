TEMPLATE = app
LANGUAGE = C++
TARGET   = common

win32:CONFIG += console
win32:DESTDIR = ./
macx:CONFIG  -= app_bundle

unix:CONFIG    += link_pkgconfig
unix:PKGCONFIG += libusb-1.0

HEADERS += ioenumerator.h \
           iodevice.h \
           outputdevice.h \
           inputdevice.h

SOURCES += ioenumerator.cpp \
           iodevice.cpp \
           outputdevice.cpp \
           inputdevice.cpp \
           main.cpp

unix:{
    HEADERS += unixioenumerator.h \
               unixpeperonidevice.h
    SOURCES += unixioenumerator.cpp \
               unixpeperonidevice.cpp
}

win32:{
    INCLUDEPATH += ../win32/peperoni
    HEADERS += win32ioenumerator.h \
               win32peperonidevice.h \
               ../win32/peperoni/usbdmx-dynamic.h
    SOURCES += win32ioenumerator.cpp \
               win32peperonidevice.cpp \
               ../win32/peperoni/usbdmx-dynamic.cpp
}
