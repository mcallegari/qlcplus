# Console application (no WIN32 flag): the comparison table goes to stdout
add_executable(beattrackerbench
    beattrackerbench.cpp
)
target_include_directories(beattrackerbench PRIVATE
    ../../../plugins/interfaces
    ../../audio/src
    ../../src
)

# Link the audio static library directly: the benchmark drives all three
# tracker implementations, including ones not referenced by (and therefore
# not exported from) the engine shared library
target_link_libraries(beattrackerbench PRIVATE
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    qlcplusaudio
    qlcplusengine
)
