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
