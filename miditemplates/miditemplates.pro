include(../variables.pri)

TEMPLATE = subdirs
TARGET = templates

templates.files += APC20.qxm

templates.path = $$INSTALLROOT/$$MIDITEMPLATEDIR
INSTALLS += templates
