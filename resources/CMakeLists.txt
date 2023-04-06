cmake_minimum_required(VERSION 3.16)
project(resources VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Set up AUTOMOC and some sensible defaults for runtime execution
# When using Qt 6.3, you can replace the code block below with
# qt_standard_project_setup()
set(CMAKE_AUTOMOC ON)
include(GNUInstallDirs)

find_package(QT NAMES Qt5 Qt6 REQUIRED COMPONENTS Core)


add_subdirectory(fixtures)
add_subdirectory(gobos)
add_subdirectory(inputprofiles)
add_subdirectory(miditemplates)
add_subdirectory(modifierstemplates)
add_subdirectory(rgbscripts)
add_subdirectory(samples)
if(NOT qmlui)
    add_subdirectory(docs)
endif()
if(qmlui)
    add_subdirectory(colorfilters)
    add_subdirectory(meshes)
endif()
