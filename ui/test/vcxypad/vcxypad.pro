include(../../../variables.pri)

TEMPLATE = app
LANGUAGE = C++
TARGET   = vcxypad_test

QT      += testlib gui widgets
qmlui|greaterThan(QT_MAJOR_VERSION, 5) {
  QT += qml
} else {
  QT += script
}

INCLUDEPATH += ../../../plugins/interfaces
INCLUDEPATH += ../../../engine/src
INCLUDEPATH += ../../src ../../src/virtualconsole
DEPENDPATH  += ../../src

QMAKE_LIBDIR += ../../../engine/src
QMAKE_LIBDIR += ../../src
LIBS         += -lqlcplusengine -lqlcplusui

# Test sources
SOURCES += vcxypad_test.cpp
HEADERS += vcxypad_test.h ../common/resource_paths.h
