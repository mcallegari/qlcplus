set(module_name "velleman_test")

add_executable(${module_name} WIN32
    ../../interfaces/qlcioplugin.cpp ../../interfaces/qlcioplugin.h
    ../src/velleman_mock.cpp
    ${module_name}.cpp ${module_name}.h
)
target_include_directories(${module_name} PRIVATE
    ../../interfaces
    ../src
)

target_link_libraries(${module_name} PRIVATE
    # Remove: L../src
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Test
    velleman
)

if(WIN32)
    target_compile_definitions(${module_name} PRIVATE
        QLC_EXPORT
    )
endif()

# Consider using qt_generate_deploy_app_script() for app deployment if
# the project can use Qt 6.3. In that case rerun qmake2cmake with
# --min-qt-version=6.3.
