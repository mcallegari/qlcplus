include(../../../variables.pri)

TEMPLATE = app
LANGUAGE = C++
TARGET   = audiobar_test

QT      += testlib gui widgets
qmlui|greaterThan(QT_MAJOR_VERSION, 5) {
  QT += qml
} else {
  QT += script
}

INCLUDEPATH += ../../src ../../src/virtualconsole
INCLUDEPATH += ../../../engine/src
DEPENDPATH  += ../../src

QMAKE_LIBDIR += ../../src
QMAKE_LIBDIR += ../../../engine/src
LIBS         += -lqlcplusui -lqlcplusengine

# Test sources
SOURCES += audiobar_test.cpp
HEADERS += audiobar_test.h
