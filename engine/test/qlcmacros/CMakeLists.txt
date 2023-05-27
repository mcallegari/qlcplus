add_executable(qlcmacros_test WIN32
    qlcmacros_test.cpp qlcmacros_test.h
)
target_include_directories(qlcmacros_test PRIVATE
    ../../../plugins/interfaces
    ../../src
)

target_link_libraries(qlcmacros_test PRIVATE
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::Test
    qlcplusengine
)

# Consider using qt_generate_deploy_app_script() for app deployment if
# the project can use Qt 6.3. In that case rerun qmake2cmake with
# --min-qt-version=6.3.
