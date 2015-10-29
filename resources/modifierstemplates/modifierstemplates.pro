include(../../variables.pri)

TEMPLATE = subdirs
TARGET = modtemplates

modtemplates.files += *.qxmt

modtemplates.path = $$INSTALLROOT/$$MODIFIERSTEMPLATEDIR
INSTALLS += modtemplates
