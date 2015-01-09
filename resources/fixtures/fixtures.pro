include(../../variables.pri)

TEMPLATE = subdirs
TARGET = fixtures

fixtures.files += FixturesMap.xml
fixtures.files += *.qxf

fixtures.path = $$INSTALLROOT/$$FIXTUREDIR
INSTALLS += fixtures
