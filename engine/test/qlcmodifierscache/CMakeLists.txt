add_executable(qlcmodifierscache_test WIN32
    qlcmodifierscache_test.cpp qlcmodifierscache_test.h
)
target_include_directories(qlcmodifierscache_test PRIVATE
    ../../../plugins/interfaces
    ../../src
)

target_link_libraries(qlcmodifierscache_test PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Test
    qlcplusengine
)

# Consider using qt_generate_deploy_app_script() for app deployment if
# the project can use Qt 6.3. In that case rerun qmake2cmake with
# --min-qt-version=6.3.
