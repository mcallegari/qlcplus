include(../../variables.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = idn

QT      += network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

INCLUDEPATH += ../interfaces
INCLUDEPATH += $$PWD
CONFIG      += plugin
CONFIG   += console

target.path = $$INSTALLROOT/$$PLUGINDIR
INSTALLS   += target

TRANSLATIONS += IDN_de_DE.ts

HEADERS += ../interfaces/qlcioplugin.h
HEADERS +=  idnconfiguration.h \
            idnclient.h \
            idncontroller.h \
            idnpacketizer.h \
            idnoptimizer.h \
            idn.h

SOURCES += ../interfaces/qlcioplugin.cpp
SOURCES +=  idnconfiguration.cpp \
            idnclient.cpp \
            idncontroller.cpp \
            idnpacketizer.cpp \
            idnoptimizer.cpp \
            idn.cpp

metainfo.path   = $$INSTALLROOT/share/appdata/ 
metainfo.files += qlcplus-idn.metainfo.xml
INSTALLS       += metainfo

FORMS += \
    idnconfiguration.ui

win32{
	DEFINES  += _WIN32
	LIBS += -lws2_32
}

unix{
      DEFINES += _UNIX
}


