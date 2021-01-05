include(../../variables.pri)

TEMPLATE = subdirs
TARGET = colorfilters

colorfilters.files += *.qxcf

colorfilters.path = $$INSTALLROOT/$$COLORFILTERSDIR
INSTALLS += colorfilters
