project(fixture_meshes)

set(FIXTURE_MESH_FILES *.dae)

install(FILES ${FIXTURE_MESH_FILES} DESTINATION ${INSTALLROOT}/${MESHESDIR}/fixtures)