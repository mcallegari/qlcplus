include(../../../variables.pri)
include(../../../coverage.pri)
TEMPLATE = app
LANGUAGE = C++
TARGET   = mastertimer_test

QT      += testlib
CONFIG  -= app_bundle

DEPENDPATH   += ../../src
INCLUDEPATH  += ../../../plugins/interfaces
INCLUDEPATH  += ../function
INCLUDEPATH  += ../../src
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

SOURCES += ../../../plugins/interfaces/qlcioplugin.cpp
SOURCES += mastertimer_test.cpp dmxsource_stub.cpp ../function/function_stub.cpp
HEADERS += mastertimer_test.h dmxsource_stub.h ../function/function_stub.h ../common/resource_paths.h

