# Copy the QLC+ data files into the iOS app bundle, following the
# same layout as the install rules (see the iOS paths in variables.cmake).
#
# Usage: cmake -DSOURCE_DIR=<qlcplus source> -DBUNDLE_DIR=<.app dir> -P bundle-resources.cmake

set(RES "${SOURCE_DIR}/resources")

function(copy_files dest)
    file(MAKE_DIRECTORY "${BUNDLE_DIR}/${dest}")
    file(COPY ${ARGN} DESTINATION "${BUNDLE_DIR}/${dest}")
endfunction()

# Fixtures: vendor folders + FixturesMap.xml
file(GLOB fixture_entries LIST_DIRECTORIES true "${RES}/fixtures/*")
foreach(entry IN LISTS fixture_entries)
    if(IS_DIRECTORY "${entry}" AND NOT entry MATCHES "/scripts$")
        copy_files("Fixtures" "${entry}")
    endif()
endforeach()
copy_files("Fixtures" "${RES}/fixtures/FixturesMap.xml")

# Gobos
file(GLOB gobo_dirs LIST_DIRECTORIES true "${RES}/gobos/*")
list(FILTER gobo_dirs EXCLUDE REGEX "CMakeLists.txt$")
copy_files("Gobos" ${gobo_dirs})

file(GLOB files "${RES}/inputprofiles/*.qxi")
copy_files("InputProfiles" ${files})

file(GLOB files "${RES}/miditemplates/*.qxm")
copy_files("MidiTemplates" ${files})

file(GLOB files "${RES}/modifierstemplates/*.qxmt")
copy_files("ModifiersTemplates" ${files})

file(GLOB files "${RES}/rgbscripts/*.js")
list(FILTER files EXCLUDE REGEX "/empty\\.js$")
copy_files("RGBScripts" ${files})

file(GLOB files "${RES}/colorfilters/*.qxcf")
copy_files("ColorFilters" ${files})

file(GLOB files "${RES}/meshes/fixtures/*.dae")
copy_files("Meshes/fixtures" ${files})
file(GLOB files "${RES}/meshes/generic/*.obj")
copy_files("Meshes/generic" ${files})
file(GLOB files "${RES}/meshes/stage/*.obj")
copy_files("Meshes/stage" ${files})

# Samples and translations go in the bundle root
file(GLOB files "${RES}/samples/*.qxw")
copy_files("" ${files})

file(GLOB files "${SOURCE_DIR}/qlcplus_*.qm")
if(files)
    copy_files("" ${files})
endif()

# Web access files (QML UI set)
set(web "${SOURCE_DIR}/webaccess/res")
copy_files("Web"
    "${web}/common.css" "${web}/keypad.html" "${web}/networkconfig.js"
    "${web}/configuration.js" "${web}/websocket.js"
    "${web}/favicon.ico" "${web}/favicon-192x192.png"
    "${web}/simpledesk-v5.css" "${web}/simpledesk-v5.js"
    "${web}/webaccess-v5.css" "${web}/webaccess-v5.js" "${web}/webaccess-v5.html"
)
