project(fixtures)

# Install FixturesMap.xml and all subdirectories (except ./scripts and .)
file(GLOB FIXTURES_FILES_AND_DIRECTORIES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*)
list(FILTER FIXTURES_FILES_AND_DIRECTORIES EXCLUDE REGEX "^scripts$")
list(REMOVE_ITEM FIXTURES_FILES_AND_DIRECTORIES ".")

# Only perform the copy if we're building for Android
if(ANDROID)
    # Loop through each file and directory and copy them to the assets directory, excluding the scripts directory
    foreach(FIXTURE_FILE_AND_DIRECTORY ${FIXTURES_FILES_AND_DIRECTORIES})
        if(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${FIXTURE_FILE_AND_DIRECTORY}")
            # Calculate the full path to the source directory
            set(SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${FIXTURE_FILE_AND_DIRECTORY}")

            # Calculate the full path to the destination directory within the Android package
            set(DESTINATION_DIR "${ANDROID_PACKAGE_SOURCE_DIR}/${FIXTUREDIR}")

            # Copy the directory to the assets directory
            file(COPY ${SOURCE_DIR} DESTINATION ${DESTINATION_DIR})
        endif()
    endforeach()

    # Copy the FixturesMap.xml file to the assets directory
    set(SOURCE_FIXTURES_MAP "${CMAKE_CURRENT_SOURCE_DIR}/FixturesMap.xml")
    set(DESTINATION_FIXTURES_MAP "${ANDROID_PACKAGE_SOURCE_DIR}/${FIXTUREDIR}")
    file(COPY ${SOURCE_FIXTURES_MAP} DESTINATION ${DESTINATION_FIXTURES_MAP})
endif()

foreach(FIXTURE_FILE_AND_DIRECTORY ${FIXTURES_FILES_AND_DIRECTORIES})
    if(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${FIXTURE_FILE_AND_DIRECTORY}")
        install(DIRECTORY ${FIXTURE_FILE_AND_DIRECTORY} DESTINATION ${INSTALLROOT}/${FIXTUREDIR})
    endif()
endforeach()

install(FILES FixturesMap.xml DESTINATION ${INSTALLROOT}/${FIXTUREDIR})