project(scripts)

file(GLOB SCRIPT_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.js")
list(REMOVE_ITEM SCRIPT_FILES "empty.js")

# Only perform the copy if we're building for Android
if(ANDROID)
    # Define the destination directory for the script files within the Android package
    set(SCRIPTS_ASSETS_DIR "${QT_ANDROID_PACKAGE_SOURCE_DIR}/${INSTALLROOT}/${RGBSCRIPTDIR}")

    # Create the scripts assets directory if it doesn't exist
    file(MAKE_DIRECTORY ${SCRIPTS_ASSETS_DIR})

    # Copy the script files to the assets directory
    foreach(SCRIPT_FILE IN LISTS SCRIPT_FILES)
        file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/${SCRIPT_FILE} DESTINATION ${SCRIPTS_ASSETS_DIR})
    endforeach()
endif()

install(FILES ${SCRIPT_FILES} DESTINATION ${INSTALLROOT}/${RGBSCRIPTDIR})
