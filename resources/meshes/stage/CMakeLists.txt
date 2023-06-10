project(stage_meshes)

set(STAGE_MESH_FILES *.obj)

install(FILES ${STAGE_MESH_FILES} DESTINATION ${INSTALLROOT}/${MESHESDIR}/stage)