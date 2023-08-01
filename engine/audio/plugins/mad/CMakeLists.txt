if(NOT MAD_INCLUDE_DIRS)
    find_path(MAD_INCLUDE_DIR mad.h PATH_SUFFIXES include)
    if(NOT MAD_INCLUDE_DIR)
        message(FATAL_ERROR "[ERROR] Could not find mad.h header file!")
    endif()
    set(MAD_INCLUDE_DIRS ${MAD_INCLUDE_DIR})
endif()

set(module_name "madplugin")

add_library(${module_name} SHARED)
target_sources(${module_name} PRIVATE
    ../../src/audiodecoder.cpp ../../src/audiodecoder.h
    ../../src/audioparameters.cpp ../../src/audioparameters.h
    audiodecoder_mad.cpp audiodecoder_mad.h
)

target_include_directories(${module_name} PRIVATE
    ../../src
    ${MAD_INCLUDE_DIRS}
)

target_link_libraries(${module_name} PRIVATE
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    ${MAD_LINK_LIBRARIES}
)

install(TARGETS ${module_name}
    LIBRARY DESTINATION ${INSTALLROOT}/${AUDIOPLUGINDIR}
    RUNTIME DESTINATION ${INSTALLROOT}/${AUDIOPLUGINDIR}
)