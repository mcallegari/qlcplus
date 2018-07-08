include(../../variables.pri)

TEMPLATE = subdirs
TARGET = profiles

profiles.files += *.qxi

profiles.path = $$INSTALLROOT/$$INPUTPROFILEDIR
INSTALLS += profiles
