project(windows)
# Set QT version
if(QT_MAJOR_VERSION GREATER 5)
    set(QT_V "Qt6")
    set(QT_P "qt6")
else()
    set(QT_V "Qt5")
    set(QT_P "qt5")
endif()

# Set debug mode
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(QT_D "d")
else()
    set(QT_D "")
endif()

# Set paths
# message("${QT_LIBRARY_DIR}")
get_filename_component(QT_LIBS_PATH ${QT_DIR}/../../../bin ABSOLUTE)
get_filename_component(QT_PLUGINS_PATH ${QT_LIBS_PATH}/../share/${QT_P}/plugins ABSOLUTE)
get_filename_component(QT_QML_PATH ${QT_LIBS_PATH}/../share/${QT_P}/qml ABSOLUTE)
set(SYS_LIBS_PATH $ENV{SystemDrive}/msys64/mingw32/bin)
# set(SYS_LIBS_PATH D:/msys64/mingw32/bin)

# Qt library dependencies
set(qtdeps_path "${INSTALLROOT}/${LIBSDIR}")
set(qtdeps_files
    "${SYS_LIBS_PATH}/libbz2-1.dll"
    "${SYS_LIBS_PATH}/libfreetype-6.dll"
    "${SYS_LIBS_PATH}/libglib-2.0-0.dll"
    "${SYS_LIBS_PATH}/libgraphite2.dll"
    "${SYS_LIBS_PATH}/libharfbuzz-0.dll"
    "${SYS_LIBS_PATH}/libdouble-conversion.dll"
    "${SYS_LIBS_PATH}/libiconv-2.dll"
    "${SYS_LIBS_PATH}/libintl-8.dll"
    "${SYS_LIBS_PATH}/libpcre2-8-0.dll"
    "${SYS_LIBS_PATH}/libpcre2-16-0.dll"
    "${SYS_LIBS_PATH}/libpcre-1.dll"
    "${SYS_LIBS_PATH}/libpng16-16.dll"
    "${SYS_LIBS_PATH}/libjpeg-8.dll"
    "${SYS_LIBS_PATH}/libspeex-1.dll"
    "${SYS_LIBS_PATH}/libzstd.dll"
    "${SYS_LIBS_PATH}/libbrotlidec.dll"
    "${SYS_LIBS_PATH}/libbrotlicommon.dll"
    "${SYS_LIBS_PATH}/zlib1.dll"
)

if(qmlui)
	list(APPEND qtdeps_files 
		"${SYS_LIBS_PATH}/libassimp-5.dll"
		"${SYS_LIBS_PATH}/libminizip-1.dll")
endif()

if(Qt5_MAJOR_VERSION GREATER 5)
    list(APPEND qtdeps_files "${SYS_LIBS_PATH}/libb2-1.dll")
endif()
install(FILES ${qtdeps_files} OPTIONAL DESTINATION ${qtdeps_path})

set(qtlibs_path "${INSTALLROOT}/${LIBSDIR}")
set(qtlibs_files
    "${QT_LIBS_PATH}/${QT_V}Core${QT_D}.dll"
    "${QT_LIBS_PATH}/${QT_V}Script${QT_D}.dll"
    "${QT_LIBS_PATH}/${QT_V}Network${QT_D}.dll"
    "${QT_LIBS_PATH}/${QT_V}Gui${QT_D}.dll"
    "${QT_LIBS_PATH}/${QT_V}Svg${QT_D}.dll"
    "${QT_LIBS_PATH}/${QT_V}Widgets${QT_D}.dll"
    "${QT_LIBS_PATH}/${QT_V}OpenGL${QT_D}.dll"
    "${QT_LIBS_PATH}/${QT_V}Multimedia${QT_D}.dll"
    "${QT_LIBS_PATH}/${QT_V}MultimediaWidgets${QT_D}.dll"
    "${QT_LIBS_PATH}/${QT_V}SerialPort${QT_D}.dll"
)
if(qmlui)
    list(APPEND qtlibs_files
        "${QT_LIBS_PATH}/${QT_V}Qml${QT_D}.dll"
        "${QT_LIBS_PATH}/${QT_V}QmlModels${QT_D}.dll"
        "${QT_LIBS_PATH}/${QT_V}QmlWorkerScript${QT_D}.dll"
        "${QT_LIBS_PATH}/${QT_V}Quick${QT_D}.dll"
        "${QT_LIBS_PATH}/${QT_V}QuickControls2${QT_D}.dll"
        "${QT_LIBS_PATH}/${QT_V}QuickTemplates2${QT_D}.dll"
        "${QT_LIBS_PATH}/${QT_V}Sql${QT_D}.dll"
        "${QT_LIBS_PATH}/${QT_V}3DCore${QT_D}.dll"
        "${QT_LIBS_PATH}/${QT_V}3DExtras${QT_D}.dll"
        "${QT_LIBS_PATH}/${QT_V}3DInput${QT_D}.dll"
        "${QT_LIBS_PATH}/${QT_V}3DLogic${QT_D}.dll"
        "${QT_LIBS_PATH}/${QT_V}3DAnimation${QT_D}.dll"
        "${QT_LIBS_PATH}/${QT_V}3DQuick${QT_D}.dll"
        "${QT_LIBS_PATH}/${QT_V}3DQuickExtras${QT_D}.dll"
        "${QT_LIBS_PATH}/${QT_V}3DQuickInput${QT_D}.dll"
        "${QT_LIBS_PATH}/${QT_V}3DQuickAnimation${QT_D}.dll"
        "${QT_LIBS_PATH}/${QT_V}3DQuickRender${QT_D}.dll"
        "${QT_LIBS_PATH}/${QT_V}3DRender${QT_D}.dll"
        "${QT_LIBS_PATH}/${QT_V}Concurrent${QT_D}.dll"
        "${QT_LIBS_PATH}/${QT_V}Gamepad${QT_D}.dll"
        "${QT_LIBS_PATH}/${QT_V}PrintSupport${QT_D}.dll"
        "${QT_LIBS_PATH}/${QT_V}MultimediaQuick${QT_D}.dll"
    )
endif()
install(FILES ${qtlibs_files} DESTINATION ${qtlibs_path})

# Qt platform plugin
set(qtplatform_path "${INSTALLROOT}/${LIBSDIR}/platforms")
set(qtplatform_files "${QT_PLUGINS_PATH}/platforms/qwindows${QT_D}.dll")
install(FILES ${qtplatform_files} DESTINATION ${qtplatform_path})

# Qt styles plugin
set(qtstyles_path "${INSTALLROOT}/${LIBSDIR}/styles")
set(qtstyles_files "${QT_PLUGINS_PATH}/styles/qwindowsvistastyle${QT_D}.dll")
install(FILES ${qtstyles_files} DESTINATION ${qtstyles_path})

if(Qt5_MAJOR_VERSION GREATER 5)
    set(qtmedia_path "${INSTALLROOT}/${LIBSDIR}/multimedia")
    set(qtmedia_files "${QT_PLUGINS_PATH}/multimedia/ffmpegmediaplugin${QT_D}.dll"
                        "${QT_PLUGINS_PATH}/multimedia/windowsmediaplugin${QT_D}.dll")
    install(FILES ${qtmedia_files} DESTINATION ${qtmedia_path})
else()
    set(qtaudio_path "${INSTALLROOT}/${LIBSDIR}/audio")
    set(qtaudio_files "${QT_PLUGINS_PATH}/audio/qtaudio_windows${QT_D}.dll")
    install(FILES ${qtaudio_files} DESTINATION ${qtaudio_path})

    set(qtmedia_path "${INSTALLROOT}/${LIBSDIR}/mediaservice")
    set(qtmedia_files "${QT_PLUGINS_PATH}/mediaservice/dsengine${QT_D}.dll"
                        "${QT_PLUGINS_PATH}/mediaservice/qtmedia_audioengine${QT_D}.dll")
    install(FILES ${qtmedia_files} DESTINATION ${qtmedia_path})
endif()

set(qtimageformats_path "${INSTALLROOT}/${LIBSDIR}/imageformats")
set(qtimageformats_files "${QT_PLUGINS_PATH}/imageformats/qgif${QT_D}.dll"
                          "${QT_PLUGINS_PATH}/imageformats/qjpeg${QT_D}.dll"
                          "${QT_PLUGINS_PATH}/imageformats/qsvg${QT_D}.dll")
install(FILES ${qtimageformats_files} DESTINATION ${qtimageformats_path})

if(qmlui)
    set(qtprintsupport_path "${INSTALLROOT}/${LIBSDIR}/printsupport")
    set(qtprintsupport_files "${QT_PLUGINS_PATH}/printsupport/windowsprintersupport${QT_D}.dll")
    install(FILES ${qtprintsupport_files} DESTINATION ${qtprintsupport_path})

    set(geometryloaders_path "${INSTALLROOT}/${LIBSDIR}/geometryloaders")
    set(geometryloaders_files "${QT_PLUGINS_PATH}/geometryloaders/defaultgeometryloader${QT_D}.dll")
    install(FILES ${geometryloaders_files} DESTINATION ${geometryloaders_path})

    set(sceneparsers_path "${INSTALLROOT}/${LIBSDIR}/sceneparsers")
    set(sceneparsers_files "${QT_PLUGINS_PATH}/sceneparsers/assimpsceneimport${QT_D}.dll")
    install(FILES ${sceneparsers_files} DESTINATION ${sceneparsers_path})

    set(renderers_path "${INSTALLROOT}/${LIBSDIR}/renderers")
    set(renderers_files "${QT_PLUGINS_PATH}/renderers/openglrenderer${QT_D}.dll")
    install(FILES ${renderers_files} DESTINATION ${renderers_path})

    set(qmldeps_path "${INSTALLROOT}/${LIBSDIR}")
    set(qmldeps_files "${QT_QML_PATH}/Qt"
                       "${QT_QML_PATH}/QtQml"
                       "${QT_QML_PATH}/QtQuick"
                       "${QT_QML_PATH}/QtQuick.2"
                       "${QT_QML_PATH}/Qt3D"
                       "${QT_QML_PATH}/QtMultimedia")
    install(DIRECTORY ${qmldeps_files} DESTINATION ${qmldeps_path})

    set(qmlpostinstall_commands "
        cd ${INSTALLROOT}/${LIBSDIR} && \
        find . -name plugins.qmltypes -type f -delete && \
        find . -name *.qmlc -type f -delete && \
        rm -rf Qt/WebSockets Qt/labs/location QtQml/RemoteObjects \
        rm -rf QtQuick/Extras QtQuick/Particles.2 QtQuick/XmlListModel \
        rm -rf QtQuick/Controls.2/designer QtQuick/Controls.2/Material \
        rm -rf QtQuick/Controls.2/Universal QtQuick/Controls.2/Fusion \
        rm -rf QtQuick/Controls.2/Imagine QtQuick/Scene2D
    ")
    install(CODE "execute_process(COMMAND ${qmlpostinstall_commands})")
endif()

# MSYS2 libraries
set(msys_path "${INSTALLROOT}/${LIBSDIR}")
set(msys_files "${SYS_LIBS_PATH}/libstdc++-6.dll"
                "${SYS_LIBS_PATH}/libgcc_s_dw2-1.dll"
                "${SYS_LIBS_PATH}/libwinpthread-1.dll"
                "${SYS_LIBS_PATH}/libicuin73.dll"
                "${SYS_LIBS_PATH}/libicuuc73.dll"
                "${SYS_LIBS_PATH}/libicudt73.dll"
                "${SYS_LIBS_PATH}/libmd4c.dll"
                "${SYS_LIBS_PATH}/libusb-1.0.dll")
install(FILES ${msys_files} DESTINATION ${msys_path})

# audio libraries
set(audio_path "${INSTALLROOT}/${LIBSDIR}")
if(EXISTS "${SYS_LIBS_PATH}/libmad-0.dll")
    list(APPEND audio_files "${SYS_LIBS_PATH}/libmad-0.dll")
endif()
if(EXISTS "${SYS_LIBS_PATH}/libogg-0.dll")
    list(APPEND audio_files "${SYS_LIBS_PATH}/libogg-0.dll")
endif()
if(EXISTS "${SYS_LIBS_PATH}/libopus-0.dll")
    list(APPEND audio_files "${SYS_LIBS_PATH}/libopus-0.dll")
endif()
if(EXISTS "${SYS_LIBS_PATH}/libmp3lame-0.dll")
    list(APPEND audio_files "${SYS_LIBS_PATH}/libmp3lame-0.dll")
endif()
if(EXISTS "${SYS_LIBS_PATH}/libmpg123-0.dll")
    list(APPEND audio_files "${SYS_LIBS_PATH}/libmpg123-0.dll")
endif()
if(EXISTS "${SYS_LIBS_PATH}/libvorbis-0.dll")
    list(APPEND audio_files "${SYS_LIBS_PATH}/libvorbis-0.dll")
endif()
if(EXISTS "${SYS_LIBS_PATH}/libvorbisenc-2.dll")
    list(APPEND audio_files "${SYS_LIBS_PATH}/libvorbisenc-2.dll")
endif()
if(EXISTS "${SYS_LIBS_PATH}/libFLAC.dll")
    list(APPEND audio_files "${SYS_LIBS_PATH}/libFLAC.dll")
endif()
if(EXISTS "${SYS_LIBS_PATH}/libsndfile-1.dll")
    list(APPEND audio_files "${SYS_LIBS_PATH}/libsndfile-1.dll")
endif()
if(EXISTS "${SYS_LIBS_PATH}/libfftw3-3.dll")
    list(APPEND audio_files "${SYS_LIBS_PATH}/libfftw3-3.dll")
endif()
install(FILES ${audio_files} DESTINATION ${audio_path})

# NullSoft installer files
set(nsis_path "${INSTALLROOT}/${DATADIR}")
if(qmlui)
    set(nsis_files qlcplus5${QT_V}.nsi)
else()
    set(nsis_files qlcplus4${QT_V}.nsi)
endif()
install(FILES ${nsis_files} DESTINATION ${nsis_path})

# install(FILES ../Sample.qxw DESTINATION ${INSTALLROOT}/${DATADIR})
