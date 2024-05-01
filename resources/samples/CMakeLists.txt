project(samples)

file(GLOB SAMPLE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.qxw")

# Only perform the copy if we're building for Android
if(ANDROID)
    # Define the destination directory for the sample files within the Android package
    set(SAMPLES_ASSETS_DIR "${ANDROID_PACKAGE_SOURCE_DIR}/${SAMPLESDIR}")

    # Create the samples assets directory if it doesn't exist
    file(MAKE_DIRECTORY ${SAMPLES_ASSETS_DIR})

    # Copy the sample .qxw files to the assets directory
    foreach(SAMPLE_FILE ${SAMPLE_FILES})
        file(COPY ${SAMPLE_FILE} DESTINATION ${SAMPLES_ASSETS_DIR})
    endforeach()
endif()

install(FILES ${SAMPLE_FILES} DESTINATION ${SAMPLESDIR})
