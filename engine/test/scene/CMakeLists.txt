add_executable(scene_test WIN32
    ../common/resource_paths.h
    ../mastertimer/mastertimer_stub.cpp ../mastertimer/mastertimer_stub.h
    scene_test.cpp scene_test.h
)
target_include_directories(scene_test PRIVATE
    ../../../plugins/interfaces
    ../../src
    ../iopluginstub
    ../mastertimer
)

target_link_libraries(scene_test PRIVATE
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::Test
    qlcplusengine
)

# Consider using qt_generate_deploy_app_script() for app deployment if
# the project can use Qt 6.3. In that case rerun qmake2cmake with
# --min-qt-version=6.3.
