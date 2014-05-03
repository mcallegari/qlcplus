include(../variables.pri)

TEMPLATE = subdirs
TARGET = templates

templates.files += APC20.qxm
templates.files += APC40.qxm

templates.path = $$INSTALLROOT/$$MIDITEMPLATEDIR
INSTALLS += templates
