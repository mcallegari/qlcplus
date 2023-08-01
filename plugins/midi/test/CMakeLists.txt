add_executable(midi_test WIN32 MACOSX_BUNDLE
    ../../interfaces/qlcioplugin.cpp ../../interfaces/qlcioplugin.h
    ../src/common/midiprotocol.cpp ../src/common/midiprotocol.h
    midi_test.cpp midi_test.h
)
target_include_directories(midi_test PRIVATE
    ../../interfaces
    ../src/common
)

target_link_libraries(midi_test PRIVATE
    # Remove: L../src
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Test
)

# Consider using qt_generate_deploy_app_script() for app deployment if
# the project can use Qt 6.3. In that case rerun qmake2cmake with
# --min-qt-version=6.3.
