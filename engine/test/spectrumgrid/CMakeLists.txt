add_executable(spectrumgrid_test WIN32
    spectrumgrid_test.cpp spectrumgrid_test.h
)
target_include_directories(spectrumgrid_test PRIVATE
    ../../../plugins/interfaces
    ../../audio/src
)

target_link_libraries(spectrumgrid_test PRIVATE
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Test
    qlcplusaudio
)
