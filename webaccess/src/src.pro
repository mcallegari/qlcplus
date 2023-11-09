include(../../variables.pri)
include(../../coverage.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = qlcpluswebaccess

CONFIG += qt
QT     += core gui network
QT     += widgets multimedia
qmlui|greaterThan(QT_MAJOR_VERSION, 5) {
  QT += qml
} else {
  QT += script
}

INCLUDEPATH     += qhttpserver QtWebSockets
INCLUDEPATH     += ../../engine/src ../../engine/audio/src
INCLUDEPATH     += ../../ui/src ../../ui/src/virtualconsole
DEPENDPATH      += ../../engine/src ../../ui/src
QMAKE_LIBDIR    += ../../engine/src ../../ui/src
DEFINES         += USE_WEBSOCKET NO_SSL

lessThan(QT_MAJOR_VERSION, 5) {
  macx {
    CONFIG += link_pkgconfig
    system(pkg-config --exists portaudio-2.0) {
      PKGCONFIG += portaudio-2.0
    }
  }
}

LIBS += -lqlcplusengine -lqlcplusui

win32:LIBS  += -lws2_32
win32:QMAKE_LFLAGS += -shared
win32:INCLUDEPATH += ./

# qhttpserver files
HEADERS = qhttpserver/http_parser.h \
          qhttpserver/qhttpconnection.h \
          qhttpserver/qhttpserver.h \
          qhttpserver/qhttprequest.h \
          qhttpserver/qhttpresponse.h \
          qhttpserver/qhttpserverfwd.h

SOURCES = qhttpserver/http_parser.c \
          qhttpserver/qhttpconnection.cpp \
          qhttpserver/qhttprequest.cpp \
          qhttpserver/qhttpresponse.cpp \
          qhttpserver/qhttpserver.cpp

# WebSocket protocol parsing files
# These are copied from QtWebSockets v5.15.4, with one patch to qwebsocketprotocol_p.h
# This version is selected because it added maxAllowedMessageSize, and doesn't
# use the QStringView::toInt function, used in the v6 version of the
# library, but not available to versions <5.15.2 that we still target.
HEADERS += QtWebSockets/qwebsocketdataprocessor_p.h \
           QtWebSockets/qwebsocketframe_p.h \
           QtWebSockets/qwebsocketprotocol.h \
           QtWebSockets/qwebsocketprotocol_p.h

SOURCES += QtWebSockets/qwebsocketdataprocessor.cpp \
           QtWebSockets/qwebsocketframe.cpp \
           QtWebSockets/qwebsocketprotocol.cpp

# QLC+ webaccess files
HEADERS += commonjscss.h \
           webaccess.h \
           webaccessconfiguration.h \
           webaccesssimpledesk.h \
           webaccessauth.h

unix:!macx: HEADERS += webaccessnetwork.h

SOURCES += webaccess.cpp \
           webaccessconfiguration.cpp \
           webaccesssimpledesk.cpp \
           webaccessauth.cpp

unix:!macx: SOURCES += webaccessnetwork.cpp

TRANSLATIONS += webaccess_fi_FI.ts
TRANSLATIONS += webaccess_de_DE.ts
TRANSLATIONS += webaccess_fr_FR.ts
TRANSLATIONS += webaccess_es_ES.ts
TRANSLATIONS += webaccess_it_IT.ts
TRANSLATIONS += webaccess_nl_NL.ts
TRANSLATIONS += webaccess_cz_CZ.ts
TRANSLATIONS += webaccess_pt_BR.ts
TRANSLATIONS += webaccess_ca_ES.ts
TRANSLATIONS += webaccess_ja_JP.ts

macx {
    # This must be after "TARGET = " and before target installation so that
    # install_name_tool can be run before target installation
    include(../../platforms/macos/nametool.pri)
}

target.path = $$INSTALLROOT/$$LIBSDIR
INSTALLS   += target
