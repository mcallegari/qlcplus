project(test)

# The QML UI is built as a single executable, so the sources under test are
# compiled here into a static library shared by all the test units.
# Classes that would pull in the whole application (Tardis, ContextManager,
# FixtureManager, App) are replaced by the stubs in common/qmluistubs.cpp.
set(QMLUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)

add_library(qmluitestlib STATIC
    common/qmluistubs.cpp
    common/vctestenv.h
    ${QMLUI_DIR}/tardis/tardis.h
    ${QMLUI_DIR}/tardis/simplecrypt.cpp ${QMLUI_DIR}/tardis/simplecrypt.h
    ${QMLUI_DIR}/chasereditor.cpp ${QMLUI_DIR}/chasereditor.h
    ${QMLUI_DIR}/functioneditor.cpp ${QMLUI_DIR}/functioneditor.h
    ${QMLUI_DIR}/listmodel.cpp ${QMLUI_DIR}/listmodel.h
    ${QMLUI_DIR}/previewcontext.cpp ${QMLUI_DIR}/previewcontext.h
    ${QMLUI_DIR}/treemodel.cpp ${QMLUI_DIR}/treemodel.h
    ${QMLUI_DIR}/treemodelitem.cpp ${QMLUI_DIR}/treemodelitem.h
    ${QMLUI_DIR}/virtualconsole/vcanimation.cpp ${QMLUI_DIR}/virtualconsole/vcanimation.h
    ${QMLUI_DIR}/virtualconsole/vcanimationpreset.cpp ${QMLUI_DIR}/virtualconsole/vcanimationpreset.h
    ${QMLUI_DIR}/virtualconsole/vcaudiotriggers.cpp ${QMLUI_DIR}/virtualconsole/vcaudiotriggers.h
    ${QMLUI_DIR}/virtualconsole/vcbutton.cpp ${QMLUI_DIR}/virtualconsole/vcbutton.h
    ${QMLUI_DIR}/virtualconsole/vcclock.cpp ${QMLUI_DIR}/virtualconsole/vcclock.h
    ${QMLUI_DIR}/virtualconsole/vccuelist.cpp ${QMLUI_DIR}/virtualconsole/vccuelist.h
    ${QMLUI_DIR}/virtualconsole/vcframe.cpp ${QMLUI_DIR}/virtualconsole/vcframe.h
    ${QMLUI_DIR}/virtualconsole/vclabel.cpp ${QMLUI_DIR}/virtualconsole/vclabel.h
    ${QMLUI_DIR}/virtualconsole/vcpage.cpp ${QMLUI_DIR}/virtualconsole/vcpage.h
    ${QMLUI_DIR}/virtualconsole/vcslider.cpp ${QMLUI_DIR}/virtualconsole/vcslider.h
    ${QMLUI_DIR}/virtualconsole/vcsoloframe.cpp ${QMLUI_DIR}/virtualconsole/vcsoloframe.h
    ${QMLUI_DIR}/virtualconsole/vcspeeddial.cpp ${QMLUI_DIR}/virtualconsole/vcspeeddial.h
    ${QMLUI_DIR}/virtualconsole/vcspeeddialpreset.cpp ${QMLUI_DIR}/virtualconsole/vcspeeddialpreset.h
    ${QMLUI_DIR}/virtualconsole/vcwidget.cpp ${QMLUI_DIR}/virtualconsole/vcwidget.h
    ${QMLUI_DIR}/virtualconsole/vcxypad.cpp ${QMLUI_DIR}/virtualconsole/vcxypad.h
    ${QMLUI_DIR}/virtualconsole/vcxypadpreset.cpp ${QMLUI_DIR}/virtualconsole/vcxypadpreset.h
    ${QMLUI_DIR}/virtualconsole/virtualconsole.cpp ${QMLUI_DIR}/virtualconsole/virtualconsole.h
)

target_include_directories(qmluitestlib PUBLIC
    ${QMLUI_DIR}/../engine/audio/src
    ${QMLUI_DIR}/../engine/src
    ${QMLUI_DIR}/../plugins/interfaces
    ${QMLUI_DIR}
    ${QMLUI_DIR}/tardis
    ${QMLUI_DIR}/virtualconsole
    ${CMAKE_CURRENT_SOURCE_DIR}/common
    ${FFTW3_INCLUDE_DIRS}
)

target_link_libraries(qmluitestlib PUBLIC
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::Multimedia
    Qt${QT_MAJOR_VERSION}::Qml
    Qt${QT_MAJOR_VERSION}::Quick
    Qt${QT_MAJOR_VERSION}::Test
    qlcplusengine
)

add_subdirectory(vcanimation)
add_subdirectory(vcaudiotriggers)
add_subdirectory(vcbutton)
add_subdirectory(vcclock)
add_subdirectory(vccuelist)
add_subdirectory(vcframe)
add_subdirectory(vclabel)
add_subdirectory(vcpresets)
add_subdirectory(vcslider)
add_subdirectory(vcsoloframe)
add_subdirectory(vcspeeddial)
add_subdirectory(vcwidget)
add_subdirectory(vcxypad)
add_subdirectory(virtualconsole)
