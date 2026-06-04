project(fixture_meshes)

if(RHI3D)
    file(GLOB FIXTURE_MESH_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/*.glb")
else()
    file(GLOB FIXTURE_MESH_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/*.dae")
endif()

# Only perform the copy if we're building for Android
if(ANDROID)
    # Define the destination directory for the fixture mesh files within the Android package
    set(FIXTURE_MESHES_ASSETS_DIR "${QT_ANDROID_PACKAGE_SOURCE_DIR}/${MESHESDIR}/fixtures")

    # Create the fixture meshes assets directory if it doesn't exist
    file(MAKE_DIRECTORY ${FIXTURE_MESHES_ASSETS_DIR})

    # Copy fixture mesh files to the assets directory
    foreach(FIXTURE_MESH_FILE IN LISTS FIXTURE_MESH_FILES)
        file(COPY ${FIXTURE_MESH_FILE} DESTINATION ${FIXTURE_MESHES_ASSETS_DIR})
    endforeach()
endif()

install(FILES ${FIXTURE_MESH_FILES} DESTINATION ${INSTALLROOT}/${MESHESDIR}/fixtures)
