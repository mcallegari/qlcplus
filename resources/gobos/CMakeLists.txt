project(gobo)

set(GOBO_FILES Chauvet ClayPaky GLP Others Robe SGM SGM-Color)

# Only perform the copy if we're building for Android
if(ANDROID)
    # Define the destination directory for the gobo files within the Android package
    set(GOBO_ASSETS_DIR "${ANDROID_PACKAGE_SOURCE_DIR}/${GOBODIR}")

    # Loop through each directory and copy it to the assets directory
    foreach(GOBO_DIR IN LISTS GOBO_FILES)
        # Calculate the full path to the source gobo directory
        set(SOURCE_GOBOS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${GOBO_DIR}")

        # Calculate the full path to the destination gobo directory
        set(DESTINATION_GOBOS_DIR "${GOBO_ASSETS_DIR}")

        # Copy the gobo directory to the assets directory
        file(COPY ${SOURCE_GOBOS_DIR} DESTINATION ${DESTINATION_GOBOS_DIR})
    endforeach()
endif()

install(DIRECTORY ${GOBO_FILES} DESTINATION ${INSTALLROOT}/${GOBODIR})
