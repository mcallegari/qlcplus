include(../../../variables.pri)
include(../../../coverage.pri)
TEMPLATE = app
LANGUAGE = C++
TARGET   = qlcfixturedefcache_test

QT      += testlib xml script
CONFIG  -= app_bundle

DEPENDPATH   += ../../src
INCLUDEPATH  += ../../../plugins/interfaces
INCLUDEPATH  += ../../src
QMAKE_LIBDIR += ../../src
LIBS         += -lqlcplusengine

IS_TRAVIS = $$(TRAVIS)
contains(IS_TRAVIS, "true") {
    DEFINES += SKIP_TEST
}
IS_BUILDBOT = $$(USER)
contains(IS_BUILDBOT, "buildbot") {
    DEFINES += SKIP_TEST
}

SOURCES += qlcfixturedefcache_test.cpp
HEADERS += qlcfixturedefcache_test.h ../common/resource_paths.h

