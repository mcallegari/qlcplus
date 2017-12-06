include (../../../variables.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = dmxusb

CONFIG      += plugin
QT          += gui core
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
INCLUDEPATH += ../../interfaces

# Uncomment the following to use QtSerialPort before
# any other platform specific library
#CONFIG += qtserial

# Use FTD2XX by default only in Windows.
win32 {
    CONFIG += ftd2xx
    message(Building with FTD2xx support.)
}

unix: {
    CONFIG += libftdi
}

macx: {
    CONFIG += qtserial
}

CONFIG(qtserial) {
    message(Building with QtSerialport support.)
    DEFINES += QTSERIAL
    QT += serialport
}

CONFIG(ftd2xx) {
    # FTD2XX is a proprietary interface by FTDI Ltd. and would therefore taint the
    # 100% FLOSS codebase of QLC if distributed along with QLC sources. Download
    # the latest driver package from http://www.ftdichip.com/Drivers/D2XX.htm and
    # extract its contents under FTD2XXDIR below (unix: follow the instructions in
    # the package README.dat to install under /usr/local/) before compiling this
    # plugin.
    #
    # Use forward slashes '/' instead of Windows backslashes '\\' for paths here!

    win32 {
        # Windows target
        FTD2XXDIR    = C:/Qt/D2XXSDK
        LIBS        += -L$$FTD2XXDIR/i386 -lftd2xx
        LIBS     += $$FTD2XXDIR/i386/libftd2xx.a
        INCLUDEPATH += $$FTD2XXDIR
        QMAKE_LFLAGS += -shared
    }
    DEFINES     += FTD2XX
}

CONFIG(libftdi) {
    greaterThan(QT_MAJOR_VERSION, 4) {
        macx:QT_CONFIG -= no-pkg-config
    }
    packagesExist(libftdi1) {
        CONFIG      += link_pkgconfig
        PKGCONFIG   += libftdi1 libusb-1.0
        DEFINES     += LIBFTDI1
        message(Building with libFTDI1 support.)
    } else {
        packagesExist(libftdi) {
            CONFIG      += link_pkgconfig
            PKGCONFIG   += libftdi libusb
            DEFINES     += LIBFTDI
            message(Building with libFTDI support.)
        } else {
            error(Neither libftdi-0.X nor libftdi-1.X found!)
        }
    }
}

HEADERS += ../../interfaces/qlcioplugin.h

HEADERS += dmxusb.h \
           dmxusbwidget.h \
           dmxusbconfig.h \
           enttecdmxusbpro.h \
           enttecdmxusbopen.h \
           stageprofi.h \
           vinceusbdmx512.h \
           dmxinterface.h

unix|macx: HEADERS += nanodmx.h euroliteusbdmxpro.h

SOURCES += ../../interfaces/qlcioplugin.cpp
SOURCES += dmxinterface.cpp \
           dmxusb.cpp \
           dmxusbwidget.cpp \
           dmxusbconfig.cpp \
           enttecdmxusbpro.cpp \
           enttecdmxusbopen.cpp \
           stageprofi.cpp \
           vinceusbdmx512.cpp

INCLUDEPATH += ../../midi/src/common
HEADERS += ../../midi/src/common/midiprotocol.h
SOURCES += ../../midi/src/common/midiprotocol.cpp

unix|macx: SOURCES += nanodmx.cpp euroliteusbdmxpro.cpp

CONFIG(qtserial) {
    SOURCES += qtserial-interface.cpp
    HEADERS += qtserial-interface.h
}

CONFIG(ftd2xx) {
    SOURCES += ftd2xx-interface.cpp
    HEADERS += ftd2xx-interface.h
}

CONFIG(libftdi) {
    SOURCES += libftdi-interface.cpp
    HEADERS += libftdi-interface.h
}

unix:!macx {
    # Rules to make USB DMX devices readable & writable by normal users
    udev.path  = $$UDEVRULESDIR
    udev.files = z65-dmxusb.rules
    INSTALLS  += udev
    
    metainfo.path   = $$INSTALLROOT/share/appdata/
    metainfo.files += qlcplus-dmxusb.metainfo.xml
    INSTALLS       += metainfo
}

TRANSLATIONS += DMX_USB_de_DE.ts
TRANSLATIONS += DMX_USB_es_ES.ts
TRANSLATIONS += DMX_USB_fi_FI.ts
TRANSLATIONS += DMX_USB_fr_FR.ts
TRANSLATIONS += DMX_USB_it_IT.ts
TRANSLATIONS += DMX_USB_nl_NL.ts
TRANSLATIONS += DMX_USB_cz_CZ.ts
TRANSLATIONS += DMX_USB_pt_BR.ts
TRANSLATIONS += DMX_USB_ca_ES.ts
TRANSLATIONS += DMX_USB_ja_JP.ts

macx {
    # This must be after "TARGET = " and before target installation so that
    # install_name_tool can be run before target installation
    include(../../../platforms/macos/nametool.pri)
    nametool.commands += && $$pkgConfigNametool(libftdi, libftdi.1.dylib)
}

# Plugin installation
target.path = $$INSTALLROOT/$$PLUGINDIR
INSTALLS   += target
