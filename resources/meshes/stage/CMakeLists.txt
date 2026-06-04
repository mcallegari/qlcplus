project(stage_meshes)

if(RHI3D)
    file(GLOB STAGE_MESH_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/*.glb")
else()
    file(GLOB STAGE_MESH_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/*.obj")
endif()

# Only perform the copy if we're building for Android
if(ANDROID)
    # Define the destination directory for the stage mesh files within the Android package
    set(STAGE_MESHES_ASSETS_DIR "${QT_ANDROID_PACKAGE_SOURCE_DIR}/${MESHESDIR}/stage")

    # Create the stage meshes assets directory if it doesn't exist
    file(MAKE_DIRECTORY ${STAGE_MESHES_ASSETS_DIR})

    # Copy stage mesh files to the assets directory
    foreach(STAGE_MESH_FILE IN LISTS STAGE_MESH_FILES)
        file(COPY ${STAGE_MESH_FILE} DESTINATION ${STAGE_MESHES_ASSETS_DIR})
    endforeach()
endif()

install(FILES ${STAGE_MESH_FILES} DESTINATION ${INSTALLROOT}/${MESHESDIR}/stage)
