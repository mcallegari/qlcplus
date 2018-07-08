include(../../../variables.pri)

TEMPLATE = subdirs

stage_meshes.path = $$INSTALLROOT/$$MESHESDIR/stage
stage_meshes.files = *.obj

INSTALLS += stage_meshes
