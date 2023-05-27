set(module_name "sndfileplugin")

add_library(${module_name})
target_sources(${module_name} PRIVATE
    ../../src/audiodecoder.cpp ../../src/audiodecoder.h
    ../../src/audioparameters.cpp ../../src/audioparameters.h
    audiodecoder_sndfile.cpp audiodecoder_sndfile.h
)
target_include_directories(${module_name} PRIVATE
    ../../src
)

target_link_libraries(${module_name} PRIVATE
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
)

install(TARGETS ${module_name}
    DESTINATION ${INSTALLROOT}/${AUDIOPLUGINDIR}
)
