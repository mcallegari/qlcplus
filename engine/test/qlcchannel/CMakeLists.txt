add_executable(qlcchannel_test WIN32
    qlcchannel_test.cpp qlcchannel_test.h
)
target_include_directories(qlcchannel_test PRIVATE
    ../../../plugins/interfaces
    ../../src
)

target_link_libraries(qlcchannel_test PRIVATE
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::Test
    qlcplusengine
)

# Consider using qt_generate_deploy_app_script() for app deployment if
# the project can use Qt 6.3. In that case rerun qmake2cmake with
# --min-qt-version=6.3.
