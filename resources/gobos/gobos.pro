include(../../variables.pri)

TEMPLATE = subdirs
TARGET = gobo

gobo.files += Chauvet
gobo.files += ClayPaky
gobo.files += GLP
gobo.files += Others
gobo.files += Robe
gobo.files += SGM
gobo.files += SGM-Color

gobo.path = $$INSTALLROOT/$$GOBODIR
INSTALLS += gobo
