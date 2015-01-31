include(../../variables.pri)

TEMPLATE = subdirs
TARGET = miditemplates

miditemplates.files += APC20.qxm
miditemplates.files += APC40.qxm
miditemplates.files += APCmini.qxm

miditemplates.path = $$INSTALLROOT/$$MIDITEMPLATEDIR
INSTALLS += miditemplates
