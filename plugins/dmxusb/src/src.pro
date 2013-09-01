include (../../../variables.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = dmxusb

CONFIG      += plugin
QT          += gui core
INCLUDEPATH += ../../interfaces

exists( $$[QT_INSTALL_LIBS]/libQtSerialPort.a ) {
    CONFIG += serialport
}

serialport {
    message(Building with QtSerialport support.)
    DEFINES += QTSERIAL
} else {
# Use FTD2XX by default only in Windows. Uncomment the two rows with curly
# braces to use ftd2xx interface on unix.
    win32 {
        CONFIG += ftd2xx
        message(Building with FTD2xx support.)
    }

# FTD2XX is a proprietary interface by FTDI Ltd. and would therefore taint the
# 100% FLOSS codebase of QLC if distributed along with QLC sources. Download
# the latest driver package from http://www.ftdichip.com/Drivers/D2XX.htm and
# extract its contents under FTD2XXDIR below (unix: follow the instructions in
# the package's README.dat to install under /usr/local/) before compiling this
# plugin.
#
# Use forward slashes "/" instead of Windows backslashes "\" for paths here!
    CONFIG(ftd2xx) {
        win32 {
            # Windows target
            FTD2XXDIR    = C:/Qt/CDM20828
            LIBS        += -L$$FTD2XXDIR/i386 -lftd2xx
            INCLUDEPATH += $$FTD2XXDIR
            QMAKE_LFLAGS += -shared
        } else {
            # Unix target
            INCLUDEPATH += /usr/local/include
            LIBS        += -lftd2xx -L/usr/local/lib
        }
        DEFINES     += FTD2XX
    } else {
        CONFIG      += link_pkgconfig
        PKGCONFIG   += libftdi libusb
        DEFINES     += LIBFTDI
        message(Building with libFTDI support.)
    }
}

HEADERS += ../../interfaces/qlcioplugin.h

HEADERS += dmxusb.h \
           dmxusbwidget.h \
           dmxusbconfig.h \
           enttecdmxusbpro.h \
           enttecdmxusbprorx.h \
           enttecdmxusbprotx.h \
           enttecdmxusbopen.h \
           ultradmxusbprotx.h \
           dmx4all.h \
           qlcftdi.h

SOURCES += dmxusb.cpp \
           dmxusbwidget.cpp \
           dmxusbconfig.cpp \
           enttecdmxusbpro.cpp \
           enttecdmxusbprorx.cpp \
           enttecdmxusbprotx.cpp \
           enttecdmxusbopen.cpp \
           dmx4all.cpp \
           ultradmxusbprotx.cpp

serialport {
    SOURCES += qlcftdi-qtserial.cpp
} else {
    CONFIG(ftd2xx) {
        SOURCES += qlcftdi-ftd2xx.cpp
    } else {
        SOURCES += qlcftdi-libftdi.cpp
    }
}

unix:!macx {
    # Rules to make USB DMX devices readable & writable by normal users
    udev.path  = /etc/udev/rules.d
    udev.files = z65-dmxusb.rules
    INSTALLS  += udev
}

TRANSLATIONS += DMX_USB_de_DE.ts
TRANSLATIONS += DMX_USB_es_ES.ts
TRANSLATIONS += DMX_USB_fi_FI.ts
TRANSLATIONS += DMX_USB_fr_FR.ts
TRANSLATIONS += DMX_USB_it_IT.ts
TRANSLATIONS += DMX_USB_nl_NL.ts
TRANSLATIONS += DMX_USB_cz_CZ.ts

# This must be after "TARGET = " and before target installation so that
# install_name_tool can be run before target installation
macx:include(../../../macx/nametool.pri)

# Plugin installation
target.path = $$INSTALLROOT/$$PLUGINDIR
INSTALLS   += target
