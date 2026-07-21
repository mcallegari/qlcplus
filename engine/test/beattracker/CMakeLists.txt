add_executable(beattracker_test WIN32
    beattracker_test.cpp beattracker_test.h
)
target_include_directories(beattracker_test PRIVATE
    ../../../plugins/interfaces
    ../../audio/src
    ../../src
)

target_link_libraries(beattracker_test PRIVATE
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::Test
    qlcplusengine
)
