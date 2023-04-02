include(../../../variables.cmake)
include(../../../coverage.cmake)

set(module_name "qlci18n_test")

set(TS_FILES
    qlci18n_fi_FI.ts
)

if(QT_VERSION_MAJOR GREATER 5)
    qt_add_translation(QM_FILES ${TS_FILES})
else()
    qt5_add_translation(QM_FILES ${TS_FILES})
endif()

add_executable(${module_name} WIN32
    ${module_name}.cpp ${module_name}.h
    ${QM_FILES}
)

target_include_directories(${module_name} PRIVATE
    ../../../plugins/interfaces
    ../../src
)

target_link_libraries(${module_name} PRIVATE
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::Test
    qlcplusengine
)

install(TARGETS ${module_name}
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Consider using qt_generate_deploy_app_script() for app deployment if
# the project can use Qt 6.3. In that case rerun qmake2cmake with
# --min-qt-version=6.3.
