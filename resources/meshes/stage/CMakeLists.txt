project(stage_meshes)

file(GLOB STAGE_MESH_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.obj")

# Only perform the copy if we're building for Android
if(ANDROID)
    # Define the destination directory for the stage mesh files within the Android package
    set(STAGE_MESHES_ASSETS_DIR "${ANDROID_PACKAGE_SOURCE_DIR}/${MESHESDIR}/stage")

    # Create the stage meshes assets directory if it doesn't exist
    file(MAKE_DIRECTORY ${STAGE_MESHES_ASSETS_DIR})

    # Copy the stage mesh .obj files to the assets directory
    foreach(STAGE_MESH_FILE IN LISTS STAGE_MESH_FILES)
        file(COPY ${STAGE_MESH_FILE} DESTINATION ${STAGE_MESHES_ASSETS_DIR})
    endforeach()
endif()

install(FILES ${STAGE_MESH_FILES} DESTINATION ${INSTALLROOT}/${MESHESDIR}/stage)