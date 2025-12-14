include(../../../variables.pri)

TEMPLATE = app
LANGUAGE = C++
TARGET   = audiotriggerwidget_test

QT      += testlib gui widgets
qmlui|greaterThan(QT_MAJOR_VERSION, 5) {
  QT += qml
} else {
  QT += script
}

INCLUDEPATH += ../../src
INCLUDEPATH += ../../../engine/src
DEPENDPATH  += ../../src

QMAKE_LIBDIR += ../../src
QMAKE_LIBDIR += ../../../engine/src
LIBS         += -lqlcplusui -lqlcplusengine

SOURCES += audiotriggerwidget_test.cpp
HEADERS += audiotriggerwidget_test.h
