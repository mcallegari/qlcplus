include(../../../variables.pri)

TEMPLATE = subdirs

generic_meshes.path = $$INSTALLROOT/$$MESHESDIR/generic
generic_meshes.files = *.obj

INSTALLS += generic_meshes
