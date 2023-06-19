project(generic_meshes)

set(GENERIC_MESH_FILES *.obj)

install(FILES ${GENERIC_MESH_FILES} DESTINATION ${INSTALLROOT}/${MESHESDIR}/generic)