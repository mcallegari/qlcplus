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
#if(CMAKE_BUILD_TYPE STREQUAL "Debug")
#    set(QT_D "d")
#else()
#    set(QT_D "")
#endif()

# Set paths
# message("${QT_LIBRARY_DIR}")
get_filename_component(QT_LIBS_PATH ${QT_DIR}/../../../bin ABSOLUTE)
get_filename_component(QT_PLUGINS_PATH ${QT_LIBS_PATH}/../share/${QT_P}/plugins ABSOLUTE)
get_filename_component(QT_QML_PATH ${QT_LIBS_PATH}/../share/${QT_P}/qml ABSOLUTE)
set(SYS_LIBS_PATH $ENV{SystemDrive}/msys64/mingw64/bin)
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

if(QT_MAJOR_VERSION GREATER 5)
    list(APPEND qtdeps_files "${SYS_LIBS_PATH}/libb2-1.dll")
endif()
install(FILES ${qtdeps_files} OPTIONAL DESTINATION ${qtdeps_path})

set(qtlibs_path "${INSTALLROOT}/${LIBSDIR}")
set(qtlibs_files
    "${QT_LIBS_PATH}/${QT_V}Core${QT_D}.dll"
    "${QT_LIBS_PATH}/${QT_V}Network${QT_D}.dll"
    "${QT_LIBS_PATH}/${QT_V}Gui${QT_D}.dll"
    "${QT_LIBS_PATH}/${QT_V}Svg${QT_D}.dll"
    "${QT_LIBS_PATH}/${QT_V}Widgets${QT_D}.dll"
    "${QT_LIBS_PATH}/${QT_V}OpenGL${QT_D}.dll"
    "${QT_LIBS_PATH}/${QT_V}Multimedia${QT_D}.dll"
    "${QT_LIBS_PATH}/${QT_V}MultimediaWidgets${QT_D}.dll"
    "${QT_LIBS_PATH}/${QT_V}SerialPort${QT_D}.dll"
    "${QT_LIBS_PATH}/${QT_V}WebSockets${QT_D}.dll"
)
if(QT_MAJOR_VERSION GREATER 5)
    list(APPEND qtlibs_files "${QT_LIBS_PATH}/${QT_V}Qml${QT_D}.dll")
else()
    list(APPEND qtlibs_files "${QT_LIBS_PATH}/${QT_V}Script${QT_D}.dll")
endif()
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
if(QT_MAJOR_VERSION GREATER 5)
    set(qtstyles_files "${QT_PLUGINS_PATH}/styles/qmodernwindowsstyle${QT_D}.dll")
else()
    set(qtstyles_files "${QT_PLUGINS_PATH}/styles/qwindowsvistastyle${QT_D}.dll")
endif()
install(FILES ${qtstyles_files} DESTINATION ${qtstyles_path})

if(QT_MAJOR_VERSION GREATER 5)
    set(qtmedia_path "${INSTALLROOT}/${LIBSDIR}/multimedia")
    set(qtmedia_files "${QT_PLUGINS_PATH}/multimedia/ffmpegmediaplugin${QT_D}.dll")
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
                "${SYS_LIBS_PATH}/libgcc_s_seh-1.dll"
                "${SYS_LIBS_PATH}/libwinpthread-1.dll"
                "${SYS_LIBS_PATH}/libicuin75.dll"
                "${SYS_LIBS_PATH}/libicuuc75.dll"
                "${SYS_LIBS_PATH}/libicudt75.dll"
                "${SYS_LIBS_PATH}/libmd4c.dll"
                "${SYS_LIBS_PATH}/libusb-1.0.dll")
install(FILES ${msys_files} DESTINATION ${msys_path})

function(copy_system_library target libname)
	if(EXISTS ${SYS_LIBS_PATH}/${libname})
		list(APPEND audio_files ${SYS_LIBS_PATH}/${libname})
		set(${target} "${${target}}" PARENT_SCOPE)
	endif()
endfunction()

# audio libraries
set(audio_path "${INSTALLROOT}/${LIBSDIR}")
copy_system_library(audio_files "libmad-0.dll")
copy_system_library(audio_files "libogg-0.dll")
copy_system_library(audio_files "libopus-0.dll")
copy_system_library(audio_files "libmp3lame-0.dll")
copy_system_library(audio_files "libmpg123-0.dll")
copy_system_library(audio_files "libvorbis-0.dll")
copy_system_library(audio_files "libvorbisenc-2.dll")
copy_system_library(audio_files "libFLAC.dll")
copy_system_library(audio_files "libsndfile-1.dll")
copy_system_library(audio_files "libfftw3-3.dll")

if(QT_MAJOR_VERSION GREATER 5)
	copy_system_library(audio_files "avformat-61.dll")
	copy_system_library(audio_files "avutil-59.dll")
	copy_system_library(audio_files "avcodec-61.dll")
	copy_system_library(audio_files "libbluray-2.dll")
	copy_system_library(audio_files "libgme.dll")
	copy_system_library(audio_files "libgnutls-30.dll")
	copy_system_library(audio_files "libmodplug-1.dll")
	copy_system_library(audio_files "librtmp-1.dll")
	copy_system_library(audio_files "libsrt.dll")
	copy_system_library(audio_files "libssh.dll")
	copy_system_library(audio_files "libdav1d-7.dll")
	copy_system_library(audio_files "libaom.dll")
	copy_system_library(audio_files "libcairo-2.dll")
	copy_system_library(audio_files "libgobject-2.0-0.dll")
	copy_system_library(audio_files "libgsm.dll")
	copy_system_library(audio_files "libjxl_threads.dll")
	copy_system_library(audio_files "libjxl.dll")
	copy_system_library(audio_files "liblzma-5.dll")
	copy_system_library(audio_files "liblc3-1.dll")
	copy_system_library(audio_files "libxml2-2.dll")
	copy_system_library(audio_files "libva.dll")
	copy_system_library(audio_files "libvpl-2.dll")
	copy_system_library(audio_files "libva_win32.dll")
	copy_system_library(audio_files "swscale-8.dll")
	copy_system_library(audio_files "swresample-5.dll")
	copy_system_library(audio_files "libopencore-amrnb-0.dll")
	copy_system_library(audio_files "libopencore-amrwb-0.dll")
	copy_system_library(audio_files "libopenjp2-7.dll")
	copy_system_library(audio_files "rav1e.dll")
	copy_system_library(audio_files "librsvg-2-2.dll")
	copy_system_library(audio_files "libtheoradec-1.dll")
	copy_system_library(audio_files "libSvtAv1Enc-2.dll")
	copy_system_library(audio_files "libtheoraenc-1.dll")
	copy_system_library(audio_files "libvpx-1.dll")
	copy_system_library(audio_files "libwebp-7.dll")
	copy_system_library(audio_files "libwebpmux-3.dll")
	copy_system_library(audio_files "libx265-209.dll")
	copy_system_library(audio_files "libx264-164.dll")
	copy_system_library(audio_files "xvidcore.dll")
	copy_system_library(audio_files "libzvbi-0.dll")
	copy_system_library(audio_files "libbrotlienc.dll")
	copy_system_library(audio_files "libgmp-10.dll")
	copy_system_library(audio_files "libhogweed-6.dll")
	copy_system_library(audio_files "libidn2-0.dll")
	copy_system_library(audio_files "libp11-kit-0.dll")
	copy_system_library(audio_files "libtasn1-6.dll")
	copy_system_library(audio_files "libunistring-5.dll")
	copy_system_library(audio_files "libnettle-8.dll")
	copy_system_library(audio_files "libcrypto-3-x64.dll")
	copy_system_library(audio_files "libfontconfig-1.dll")
	copy_system_library(audio_files "libpixman-1-0.dll")
	copy_system_library(audio_files "libffi-8.dll")
	copy_system_library(audio_files "libhwy.dll")
	copy_system_library(audio_files "libjxl_cms.dll")
	copy_system_library(audio_files "libsoxr.dll")
	copy_system_library(audio_files "libgdk_pixbuf-2.0-0.dll")
	copy_system_library(audio_files "libcairo-gobject-2.dll")
	copy_system_library(audio_files "libgio-2.0-0.dll")
	copy_system_library(audio_files "libpango-1.0-0.dll")
	copy_system_library(audio_files "libpangocairo-1.0-0.dll")
	copy_system_library(audio_files "libsharpyuv-0.dll")
	copy_system_library(audio_files "libexpat-1.dll")
	copy_system_library(audio_files "liblcms2-2.dll")
	copy_system_library(audio_files "libgomp-1.dll")
	copy_system_library(audio_files "libgmodule-2.0-0.dll")
	copy_system_library(audio_files "libtiff-6.dll")
	copy_system_library(audio_files "libfribidi-0.dll")
	copy_system_library(audio_files "libthai-0.dll")
	copy_system_library(audio_files "libpangoft2-1.0-0.dll")
	copy_system_library(audio_files "libpangowin32-1.0-0.dll")
	copy_system_library(audio_files "libdeflate.dll")
	copy_system_library(audio_files "libjbig-0.dll")
	copy_system_library(audio_files "libLerc.dll")
	copy_system_library(audio_files "libdatrie-1.dll")
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
