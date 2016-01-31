include(../../../variables.pri)
include(../../../coverage.pri)
TEMPLATE = app
LANGUAGE = C++
TARGET   = inputoutputmap_test

QT      += testlib
CONFIG  -= app_bundle

DEPENDPATH   += ../../src
INCLUDEPATH  += ../../../plugins/interfaces
INCLUDEPATH  += ../../src
INCLUDEPATH  += ../iopluginstub
QMAKE_LIBDIR += ../../src
LIBS         += -lqlcplusengine

IS_TRAVIS = $$(TRAVIS)
contains(IS_TRAVIS, "true") {
    DEFINES += SKIP_TEST
}

CURRUSR = $$(USER)
IS_BUILDBOT = $$find(CURRUSR, "build")
count(IS_BUILDBOT, 1) {
    DEFINES += SKIP_TEST
}

SOURCES += inputoutputmap_test.cpp
HEADERS += inputoutputmap_test.h ../common/resource_paths.h

