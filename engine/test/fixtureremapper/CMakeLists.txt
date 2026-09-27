add_executable(fixtureremapper_test WIN32
    ../common/resource_paths.h
    ../mastertimer/mastertimer_stub.cpp ../mastertimer/mastertimer_stub.h
    fixtureremapper_test.cpp fixtureremapper_test.h
)
target_include_directories(fixtureremapper_test PRIVATE
    ../../../plugins/interfaces
    ../../src
    ../iopluginstub
    ../mastertimer
)

target_link_libraries(fixtureremapper_test PRIVATE
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::Test
    qlcplusengine
)
