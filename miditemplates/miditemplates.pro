include(../variables.pri)

TEMPLATE = subdirs
TARGET = templates

templates.files += APC20.qxm
templates.files += BCF2000_NI_B4_1.qxm

templates.path = $$INSTALLROOT/$$MIDITEMPLATEDIR
INSTALLS += templates
