project(fixtures)

# # Find all subdirectories (except ./scripts and .)
# file(GLOB DIRECTORIES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *)
# list(FILTER DIRECTORIES EXCLUDE REGEX "^scripts$")
# list(REMOVE_ITEM DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR})

# # Add a subdirectory for each directory found
# foreach(directory ${DIRECTORIES})
#     add_subdirectory(${directory})
# endforeach()

# Install FixturesMap.xml and all subdirectories (except ./scripts and .)
file(GLOB_RECURSE FILES_AND_DIRECTORIES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*)
list(FILTER FILES_AND_DIRECTORIES EXCLUDE REGEX "^scripts$")
list(REMOVE_ITEM FILES_AND_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR})
list(APPEND FILES_AND_DIRECTORIES FixturesMap.xml)

install(FILES ${FILES_AND_DIRECTORIES} DESTINATION ${INSTALLROOT}/${FIXTUREDIR})
