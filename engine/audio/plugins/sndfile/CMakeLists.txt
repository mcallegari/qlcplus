set(module_name "sndfileplugin")

add_library(${module_name} SHARED)
target_sources(${module_name} PRIVATE
    ../../src/audiodecoder.cpp ../../src/audiodecoder.h
    ../../src/audioparameters.cpp ../../src/audioparameters.h
    audiodecoder_sndfile.cpp audiodecoder_sndfile.h
)
target_include_directories(${module_name} PRIVATE
    ../../src
    ${SNDFILE_INCLUDE_DIRS}
)

target_link_libraries(${module_name} PRIVATE
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    ${SNDFILE_LINK_LIBRARIES}
)

install(TARGETS ${module_name}
    LIBRARY DESTINATION ${INSTALLROOT}/${AUDIOPLUGINDIR}
    RUNTIME DESTINATION ${INSTALLROOT}/${AUDIOPLUGINDIR}
)