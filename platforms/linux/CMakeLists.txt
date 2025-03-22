project(icons)

set(desktop_FILES qlcplus.desktop)
if(NOT qmlui)
    list(APPEND desktop_FILES qlcplus-fixtureeditor.desktop)
endif()
install(FILES ${desktop_FILES} DESTINATION ${INSTALLROOT}/share/applications/)

set(icons_SRCS ../../resources/icons/png/qlcplus.png)
if(NOT qmlui)
    list(APPEND icons_SRCS ../../resources/icons/png/qlcplus-fixtureeditor.png)
endif()
install(FILES ${icons_SRCS} DESTINATION ${INSTALLROOT}/share/pixmaps/)

install(FILES qlcplus.xml DESTINATION ${INSTALLROOT}/share/mime/packages)

set(appdata_FILES org.qlcplus.QLCPlus.appdata.xml)
if(NOT qmlui)
    list(APPEND appdata_FILES org.qlcplus.QLCPlusFixtureEditor.appdata.xml)
endif()
install(FILES ${appdata_FILES} DESTINATION ${METAINFODIR})

if(NOT qmlui)
    file(GLOB manpages_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.1")
    install(FILES ${manpages_FILES} DESTINATION ${INSTALLROOT}/${MANDIR})
endif()

# install(FILES ../Sample.qxw DESTINATION ${INSTALLROOT}/${DATADIR})
# install(FILES qlcplus-start.sh DESTINATION ${INSTALLROOT}/sbin)

if(appimage)
    if (QT_DIR STREQUAL "/usr/lib/x86_64-linux-gnu/cmake/Qt5")
        set(QT_LIBS_PATH "/usr/lib/x86_64-linux-gnu")
        set(QT_PLUGINS_PATH "/usr/lib/x86_64-linux-gnu/qt5/plugins")
        set(QT_QML_PATH "/usr/lib/x86_64-linux-gnu/qt5/qml")
    else()
        get_filename_component(QT_LIBS_PATH ${QT_DIR}/../../../lib ABSOLUTE)
        get_filename_component(QT_PLUGINS_PATH ${QT_LIBS_PATH}/../plugins ABSOLUTE)
        get_filename_component(QT_QML_PATH ${QT_LIBS_PATH}/../qml ABSOLUTE)
    endif()
    message("QT_LIBS_PATH ${QT_LIBS_PATH}")
    message("QT_PLUGINS_PATH ${QT_PLUGINS_PATH}")
    message("QT_QML_PATH ${QT_QML_PATH}")
    # Qt dependencies
    file(GLOB qtdeps_FILES "${QT_LIBS_PATH}/libicu*")
    install(
        FILES ${qtdeps_FILES}
        DESTINATION ${INSTALLROOT}/${LIBSDIR}
    )

    # Qt Libraries
    if (QT_VERSION_MAJOR GREATER 5)
        set(qtlibs_FILES
            ${QT_LIBS_PATH}/libQt6Core.so
            ${QT_LIBS_PATH}/libQt6Network.so
            ${QT_LIBS_PATH}/libQt6Gui.so
            ${QT_LIBS_PATH}/libQt6Qml.so
            ${QT_LIBS_PATH}/libQt6Svg.so
            ${QT_LIBS_PATH}/libQt6Widgets.so
            ${QT_LIBS_PATH}/libQt6OpenGL.so
            ${QT_LIBS_PATH}/libQt6Multimedia.so
            ${QT_LIBS_PATH}/libQt6MultimediaWidgets.so
            ${QT_LIBS_PATH}/libQt6SerialPort.so
            ${QT_LIBS_PATH}/libQt6XcbQpa.so
            ${QT_LIBS_PATH}/libQt6DBus.so
            ${QT_LIBS_PATH}/libQt5WebSockets.so.6)
    else()
        set(qtlibs_FILES
            ${QT_LIBS_PATH}/libQt5Core.so.5
            ${QT_LIBS_PATH}/libQt5Script.so.5
            ${QT_LIBS_PATH}/libQt5Network.so.5
            ${QT_LIBS_PATH}/libQt5Gui.so.5
            ${QT_LIBS_PATH}/libQt5Svg.so.5
            ${QT_LIBS_PATH}/libQt5Widgets.so.5
            ${QT_LIBS_PATH}/libQt5OpenGL.so.5
            ${QT_LIBS_PATH}/libQt5Multimedia.so.5
            ${QT_LIBS_PATH}/libQt5MultimediaWidgets.so.5
            ${QT_LIBS_PATH}/libQt5SerialPort.so.5
            ${QT_LIBS_PATH}/libQt5XcbQpa.so.5
            ${QT_LIBS_PATH}/libQt5DBus.so.5
            ${QT_LIBS_PATH}/libQt5WebSockets.so.5)
    endif()

    if(qmlui)
        list(APPEND qtlibs_FILES
            ${QT_LIBS_PATH}/libQt6MultimediaQuick.so
            ${QT_LIBS_PATH}/libQt6MultimediaGstTools.so
            ${QT_LIBS_PATH}/libQt6QmlModels.so
            ${QT_LIBS_PATH}/libQt6QmlWorkerScript.so
            ${QT_LIBS_PATH}/libQt6Quick.so
            ${QT_LIBS_PATH}/libQt6QuickControls2.so
            ${QT_LIBS_PATH}/libQt6QuickTemplates2.so
            ${QT_LIBS_PATH}/libQt63DCore.so
            ${QT_LIBS_PATH}/libQt63DExtras.so
            ${QT_LIBS_PATH}/libQt63DInput.so
            ${QT_LIBS_PATH}/libQt63DLogic.so
            ${QT_LIBS_PATH}/libQt63DAnimation.so
            ${QT_LIBS_PATH}/libQt63DQuick.so
            ${QT_LIBS_PATH}/libQt63DQuickExtras.so
            ${QT_LIBS_PATH}/libQt63DQuickInput.so
            ${QT_LIBS_PATH}/libQt63DQuickRender.so
            ${QT_LIBS_PATH}/libQt63DRender.so
            ${QT_LIBS_PATH}/libQt6Concurrent.so
            ${QT_LIBS_PATH}/libQt6Gamepad.so
            ${QT_LIBS_PATH}/libQt6PrintSupport.so)
    endif()
    
    set(_resolved_qtlibs_FILES "")
    foreach (_file ${qtlibs_FILES})
        file(GLOB _files "${_file}*")
        list(FILTER _files EXCLUDE REGEX ".debug")
        list (APPEND _resolved_qtlibs_FILES "${_files}")
    endforeach()
    install(FILES ${_resolved_qtlibs_FILES} DESTINATION ${INSTALLROOT}/${LIBSDIR})

    # Qt plugins
    if (QT_VERSION_MAJOR GREATER 5)
        set(QT_V "qt6")
    else()
        set(QT_V "qt5")
    endif()

    install(
        FILES 
            ${QT_PLUGINS_PATH}/platforms/libqlinuxfb.so
            ${QT_PLUGINS_PATH}/platforms/libqxcb.so
            ${QT_PLUGINS_PATH}/platforms/libqminimal.so
        DESTINATION ${INSTALLROOT}/${LIBSDIR}/${QT_V}/plugins/platforms
    )

    install(
        FILES
            ${QT_PLUGINS_PATH}/xcbglintegrations/libqxcb-glx-integration.so
        DESTINATION ${INSTALLROOT}/${LIBSDIR}/${QT_V}/plugins/xcbglintegrations
    )

    if (QT_VERSION_MAJOR GREATER 5)
        install(
            FILES
                ${QT_PLUGINS_PATH}/multimedia/libffmpegmediaplugin.so
                ${QT_PLUGINS_PATH}/multimedia/libgstreamermediaplugin.so
            DESTINATION ${INSTALLROOT}/${LIBSDIR}/${QT_V}/plugins/multimedia
        )
    else()
        install(
            FILES
                ${QT_PLUGINS_PATH}/audio/libqtaudio_alsa.so
                ${QT_PLUGINS_PATH}/audio/libqtmedia_pulse.so
            DESTINATION ${INSTALLROOT}/${LIBSDIR}/${QT_V}/plugins/audio
        )

        install(
            FILES
                ${QT_PLUGINS_PATH}/mediaservice/libgstaudiodecoder.so
                ${QT_PLUGINS_PATH}/mediaservice/libgstmediaplayer.so
            DESTINATION ${INSTALLROOT}/${LIBSDIR}/${QT_V}/plugins/mediaservice
        )
    endif()

    install(
        FILES
            ${QT_PLUGINS_PATH}/imageformats/libqsvg.so
        DESTINATION ${INSTALLROOT}/${LIBSDIR}/${QT_V}/plugins/imageformats
    )

    if(qmlui)
        install(
            FILES
                ${QT_PLUGINS_PATH}/printsupport/libcupsprintersupport.so
            DESTINATION ${INSTALLROOT}/${LIBSDIR}/${QT_V}/plugins/printsupport
        )

        install(
            FILES
                ${QT_PLUGINS_PATH}/sceneparsers/libassimpsceneimport.so
            OPTIONAL DESTINATION ${INSTALLROOT}/${LIBSDIR}/${QT_V}/plugins/sceneparsers
        )

        install(
            FILES
                ${QT_PLUGINS_PATH}/geometryloaders/libdefaultgeometryloader.so
            DESTINATION ${INSTALLROOT}/${LIBSDIR}/${QT_V}/plugins/geometryloaders
        )
    endif()

    if(QT_VERSION VERSION_GREATER_EQUAL 5.15.0)
        install(FILES ${QT_PLUGINS_PATH}/renderers/libopenglrenderer.so DESTINATION ${INSTALLROOT}/${LIBSDIR}/${QT_V}/plugins/renderers)
    endif()

    set(qmldeps_files
        ${QT_QML_PATH}/QtQml
        ${QT_QML_PATH}/QtQuick
        ${QT_QML_PATH}/Qt3D
        ${QT_QML_PATH}/QtMultimedia
    )
    install(DIRECTORY ${qmldeps_files} DESTINATION ${INSTALLROOT}/bin)

endif()
