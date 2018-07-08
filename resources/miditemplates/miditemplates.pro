include(../../variables.pri)

TEMPLATE = subdirs
TARGET = miditemplates

miditemplates.files += *.qxm

miditemplates.path = $$INSTALLROOT/$$MIDITEMPLATEDIR
INSTALLS += miditemplates
