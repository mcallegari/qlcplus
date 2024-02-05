include(../../../variables.pri)

TEMPLATE = app
LANGUAGE = C++
TARGET   = vcwidgetproperties_test

QT += testlib gui widgets
qmlui|greaterThan(QT_MAJOR_VERSION, 5) {
  QT += qml
} else {
  QT += script
}

INCLUDEPATH += ../../../plugins/interfaces
INCLUDEPATH += ../../src ../../src/virtualconsole
DEPENDPATH  += ../../src

QMAKE_LIBDIR += ../../src
QMAKE_LIBDIR += ../../../engine/src
LIBS         += -lqlcplusui -lqlcplusengine

# Test sources
SOURCES += vcwidgetproperties_test.cpp
HEADERS += vcwidgetproperties_test.h
