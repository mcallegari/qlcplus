project(resources)

add_subdirectory(fixtures)
add_subdirectory(gobos)
add_subdirectory(inputprofiles)
add_subdirectory(miditemplates)
add_subdirectory(modifierstemplates)
add_subdirectory(rgbscripts)
add_subdirectory(samples)
# colorfilters and meshes are only used by the QML UI, but they are installed
# unconditionally so that the shared data package is identical for QLC+ 4 and 5
add_subdirectory(colorfilters)
add_subdirectory(meshes)
