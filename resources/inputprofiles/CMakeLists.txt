project(profiles)

file(GLOB PROFILE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.qxi")

# Only perform the copy if we're building for Android
if(ANDROID)
    # Define the destination directory for the profile files within the Android package
    set(PROFILES_ASSETS_DIR "${ANDROID_PACKAGE_SOURCE_DIR}/${INPUTPROFILEDIR}")

    # Create the profiles assets directory if it doesn't exist
    file(MAKE_DIRECTORY ${PROFILES_ASSETS_DIR})

    # Copy the profile .qxi files to the assets directory
    foreach(PROFILE_FILE IN LISTS PROFILE_FILES)
        file(COPY ${PROFILE_FILE} DESTINATION ${PROFILES_ASSETS_DIR})
    endforeach()
endif()

install(FILES ${PROFILE_FILES} DESTINATION ${INSTALLROOT}/${INPUTPROFILEDIR})