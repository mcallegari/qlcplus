include(../../../variables.cmake)
include(../../../coverage.cmake)

add_executable(mastertimer_test WIN32
    # ../../../plugins/interfaces/qlcioplugin.cpp
    ../common/resource_paths.h
    ../function/function_stub.cpp ../function/function_stub.h
    dmxsource_stub.cpp dmxsource_stub.h
    mastertimer_test.cpp mastertimer_test.h
)
target_include_directories(mastertimer_test PRIVATE
    ../../../plugins/interfaces
    ../../src
    ../function
)

target_link_libraries(mastertimer_test PRIVATE
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::Test
    qlcplusengine
)

if(IS_TRAVIS___contains___ON)
    target_compile_definitions(mastertimer_test PRIVATE
        SKIP_TEST
    )
endif()

# if(count(IS_BUILDBOT, 1))
if(IS_BUILDBOT)
    target_compile_definitions(mastertimer_test PRIVATE
        SKIP_TEST
    )
endif()

install(TARGETS mastertimer_test
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Consider using qt_generate_deploy_app_script() for app deployment if
# the project can use Qt 6.3. In that case rerun qmake2cmake with
# --min-qt-version=6.3.
