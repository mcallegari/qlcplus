add_executable(tempoanalyzer_test WIN32
    tempoanalyzer_test.cpp tempoanalyzer_test.h
)
target_include_directories(tempoanalyzer_test PRIVATE
    ../../../plugins/interfaces
    ../../audio/src
    ../../src
)

target_link_libraries(tempoanalyzer_test PRIVATE
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::Test
    qlcplusengine
)
