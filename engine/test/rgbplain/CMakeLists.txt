add_executable(rgbplain_test WIN32
    rgbplain_test.cpp rgbplain_test.h
)
target_include_directories(rgbplain_test PRIVATE
    ../../../plugins/interfaces
    ../../src
)

target_link_libraries(rgbplain_test PRIVATE
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::Test
    qlcplusengine
)

# Consider using qt_generate_deploy_app_script() for app deployment if
# the project can use Qt 6.3. In that case rerun qmake2cmake with
# --min-qt-version=6.3.
