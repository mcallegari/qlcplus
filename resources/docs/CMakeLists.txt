project(docs)

file(GLOB HTML_FILES "${CMAKE_CURRENT_SOURCE_DIR}/html_en_EN/*.html")
file(GLOB IMAGE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/images/*.png")

# Only perform the copy if we're building for Android
if(ANDROID)
    # Define the destination directories for the HTML and image files within the Android package
    set(HTML_ASSETS_DIR "${ANDROID_PACKAGE_SOURCE_DIR}/${DOCSDIR}/html")
    set(IMAGES_ASSETS_DIR "${ANDROID_PACKAGE_SOURCE_DIR}/${DOCSDIR}/images")

    # Create the assets directories if they don't exist
    file(MAKE_DIRECTORY ${HTML_ASSETS_DIR})
    file(MAKE_DIRECTORY ${IMAGES_ASSETS_DIR})

    # Copy the HTML files to the assets directory
    foreach(HTML_FILE IN LISTS HTML_FILES)
        file(COPY ${HTML_FILE} DESTINATION ${HTML_ASSETS_DIR})
    endforeach()

    # Copy the image files to the assets directory
    foreach(IMAGE_FILE IN LISTS IMAGE_FILES)
        file(COPY ${IMAGE_FILE} DESTINATION ${IMAGES_ASSETS_DIR})
    endforeach()
endif()

install(FILES ${HTML_FILES} DESTINATION ${INSTALLROOT}/${DOCSDIR}/html)
install(FILES ${IMAGE_FILES} DESTINATION ${INSTALLROOT}/${DOCSDIR}/images)
