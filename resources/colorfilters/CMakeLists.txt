project(colorfilters)

file(GLOB COLOR_FILTER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.qxcf")

if(ANDROID)
    set(COLOR_FILTERS_ASSETS_DIR "${ANDROID_PACKAGE_SOURCE_DIR}/${COLORFILTERSDIR}")
    # Create the assets directory if it doesn't exist
    file(MAKE_DIRECTORY ${COLOR_FILTERS_ASSETS_DIR})

    # Copy the color filter files to the assets directory
    foreach(COLOR_FILTER_FILE ${COLOR_FILTER_FILES})
        file(COPY ${COLOR_FILTER_FILE} DESTINATION ${COLOR_FILTERS_ASSETS_DIR})
    endforeach()
endif()

install(FILES ${COLOR_FILTER_FILES} DESTINATION ${INSTALLROOT}/${COLORFILTERSDIR})
