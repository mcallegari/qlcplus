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
        ${QT_LIBS_PATH}/libQt5WebSockets.so.5
    )
    if(qmlui)
        list(APPEND qtlibs_FILES
            ${QT_LIBS_PATH}/libQt5MultimediaQuick.so.5
            ${QT_LIBS_PATH}/libQt5MultimediaGstTools.so.5
            ${QT_LIBS_PATH}/libQt5Qml.so.5
            ${QT_LIBS_PATH}/libQt5QmlModels.so.5
            ${QT_LIBS_PATH}/libQt5QmlWorkerScript.so.5
            ${QT_LIBS_PATH}/libQt5Quick.so.5
            ${QT_LIBS_PATH}/libQt5QuickControls2.so.5
            ${QT_LIBS_PATH}/libQt5QuickTemplates2.so.5
            ${QT_LIBS_PATH}/libQt53DCore.so.5
            ${QT_LIBS_PATH}/libQt53DExtras.so.5
            ${QT_LIBS_PATH}/libQt53DInput.so.5
            ${QT_LIBS_PATH}/libQt53DLogic.so.5
            ${QT_LIBS_PATH}/libQt53DAnimation.so.5
            ${QT_LIBS_PATH}/libQt53DQuick.so.5
            ${QT_LIBS_PATH}/libQt53DQuickExtras.so.5
            ${QT_LIBS_PATH}/libQt53DQuickInput.so.5
            ${QT_LIBS_PATH}/libQt53DQuickRender.so.5
            ${QT_LIBS_PATH}/libQt53DRender.so.5
            ${QT_LIBS_PATH}/libQt5Concurrent.so.5
            ${QT_LIBS_PATH}/libQt5Gamepad.so.5
            ${QT_LIBS_PATH}/libQt5PrintSupport.so.5)
    endif()
    
    set(_resolved_qtlibs_FILES "")
    foreach (_file ${qtlibs_FILES})
        file(GLOB _files "${_file}*")
        list(FILTER _files EXCLUDE REGEX ".debug")
        list (APPEND _resolved_qtlibs_FILES "${_files}")
    endforeach()
    install(FILES ${_resolved_qtlibs_FILES} DESTINATION ${INSTALLROOT}/${LIBSDIR})

    # Qt plugins
    install(
        FILES 
            ${QT_PLUGINS_PATH}/platforms/libqlinuxfb.so
            ${QT_PLUGINS_PATH}/platforms/libqxcb.so
            ${QT_PLUGINS_PATH}/platforms/libqminimal.so
        DESTINATION ${INSTALLROOT}/${LIBSDIR}/qt5/plugins/platforms
    )

    install(
        FILES
            ${QT_PLUGINS_PATH}/xcbglintegrations/libqxcb-glx-integration.so
        DESTINATION ${INSTALLROOT}/${LIBSDIR}/qt5/plugins/xcbglintegrations
    )

    install(
        FILES
            ${QT_PLUGINS_PATH}/audio/libqtaudio_alsa.so
            ${QT_PLUGINS_PATH}/audio/libqtmedia_pulse.so
        DESTINATION ${INSTALLROOT}/${LIBSDIR}/qt5/plugins/audio
    )

    install(
        FILES
            ${QT_PLUGINS_PATH}/mediaservice/libgstaudiodecoder.so
            ${QT_PLUGINS_PATH}/mediaservice/libgstmediaplayer.so
        DESTINATION ${INSTALLROOT}/${LIBSDIR}/qt5/plugins/mediaservice
    )

    install(
        FILES
            ${QT_PLUGINS_PATH}/imageformats/libqsvg.so
        DESTINATION ${INSTALLROOT}/${LIBSDIR}/qt5/plugins/imageformats
    )

    if(qmlui)
        install(
            FILES
                ${QT_PLUGINS_PATH}/printsupport/libcupsprintersupport.so
            DESTINATION ${INSTALLROOT}/${LIBSDIR}/qt5/plugins/printsupport
        )

        install(
            FILES
                ${QT_PLUGINS_PATH}/sceneparsers/libassimpsceneimport.so
            OPTIONAL DESTINATION ${INSTALLROOT}/${LIBSDIR}/qt5/plugins/sceneparsers
        )

        install(
            FILES
                ${QT_PLUGINS_PATH}/geometryloaders/libdefaultgeometryloader.so
            DESTINATION ${INSTALLROOT}/${LIBSDIR}/qt5/plugins/geometryloaders
        )
    endif()

    if(QT_VERSION VERSION_GREATER_EQUAL 5.15.0)
        install(FILES ${QT_PLUGINS_PATH}/renderers/libopenglrenderer.so DESTINATION ${INSTALLROOT}/${LIBSDIR}/qt5/plugins/renderers)
    endif()

    set(qmldeps_files
        ${QT_QML_PATH}/QtQml
        ${QT_QML_PATH}/QtQuick
        ${QT_QML_PATH}/QtQuick.2
        ${QT_QML_PATH}/Qt3D
        ${QT_QML_PATH}/QtMultimedia
    )
    install(DIRECTORY ${qmldeps_files} DESTINATION ${INSTALLROOT}/bin)

endif()
