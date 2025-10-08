project(miditemplates)

file(GLOB MIDITEMPLATE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.qxm")

# Only perform the copy if we're building for Android
if(ANDROID)
    # Define the destination directory for the miditemplate files within the Android package
    set(MIDITEMPLATES_ASSETS_DIR "${ANDROID_PACKAGE_SOURCE_DIR}/${MIDITEMPLATEDIR}")

    # Create the miditemplates assets directory if it doesn't exist
    file(MAKE_DIRECTORY ${MIDITEMPLATES_ASSETS_DIR})

    # Copy the miditemplate .qxm files to the assets directory
    foreach(MIDITEMPLATE_FILE IN LISTS MIDITEMPLATE_FILES)
        file(COPY ${MIDITEMPLATE_FILE} DESTINATION ${MIDITEMPLATES_ASSETS_DIR})
    endforeach()
endif()

install(FILES ${MIDITEMPLATE_FILES} DESTINATION ${INSTALLROOT}/${MIDITEMPLATEDIR})