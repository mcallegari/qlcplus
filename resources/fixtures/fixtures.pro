include(../../variables.pri)

TEMPLATE = subdirs
TARGET = fixtures

DIRLIST = $$system(find . -type d -not -path ./scripts -not -path .)
#message ("DIRLIST is $$DIRLIST")

fixtures.files += FixturesMap.xml
fixtures.files += $${DIRLIST}
#fixtures.files += $$system(find . -name *.qxf)

fixtures.path = $$INSTALLROOT/$$FIXTUREDIR
INSTALLS += fixtures
