include(../../../variables.pri)

TEMPLATE = subdirs

fixture_meshes.path = $$INSTALLROOT/$$MESHESDIR/fixtures
#fixture_meshes.files = *.obj
fixture_meshes.files = *.dae

INSTALLS += fixture_meshes
