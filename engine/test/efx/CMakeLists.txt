add_executable(efx_test WIN32
    ../common/resource_paths.h
    ../mastertimer/mastertimer_stub.cpp ../mastertimer/mastertimer_stub.h
    efx_test.cpp efx_test.h
)
target_include_directories(efx_test PRIVATE
    ../../../plugins/interfaces
    ../../src
    ../mastertimer
)

target_link_libraries(efx_test PRIVATE
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::Test
    qlcplusengine
)

if(APPLE)
    set_target_properties(efx_test PROPERTIES
        MACOSX_BUNDLE FALSE
    )
endif()

# Consider using qt_generate_deploy_app_script() for app deployment if
# the project can use Qt 6.3. In that case rerun qmake2cmake with
# --min-qt-version=6.3.
