project(windows)
# Set QT version
if(QT_MAJOR_VERSION GREATER 5)
    set(QT_V "Qt6")
    set(QT_P "qt6")
else()
    set(QT_V "Qt5")
    set(QT_P "qt5")
endif()

# Set paths
# message("${QT_LIBRARY_DIR}")
get_filename_component(QT_LIBS_PATH ${QT_DIR}/../../../bin ABSOLUTE)
get_filename_component(QT_PLUGINS_PATH ${QT_LIBS_PATH}/../share/${QT_P}/plugins ABSOLUTE)
get_filename_component(QT_QML_PATH ${QT_LIBS_PATH}/../share/${QT_P}/qml ABSOLUTE)
if ($ENV{MSYSTEM_CARCH} STREQUAL "i686")
    set(SYS_LIBS_PATH $ENV{SystemDrive}/msys64/mingw32/bin)
else()
    set(SYS_LIBS_PATH $ENV{SystemDrive}/msys64/mingw64/bin)
endif()
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
    "${QT_LIBS_PATH}/${QT_V}Core.dll"
    "${QT_LIBS_PATH}/${QT_V}Network.dll"
    "${QT_LIBS_PATH}/${QT_V}Gui.dll"
    "${QT_LIBS_PATH}/${QT_V}Svg.dll"
    "${QT_LIBS_PATH}/${QT_V}Widgets.dll"
    "${QT_LIBS_PATH}/${QT_V}OpenGL.dll"
    "${QT_LIBS_PATH}/${QT_V}Multimedia.dll"
    "${QT_LIBS_PATH}/${QT_V}MultimediaWidgets.dll"
    "${QT_LIBS_PATH}/${QT_V}SerialPort.dll"
    "${QT_LIBS_PATH}/${QT_V}WebSockets.dll"
)
if(QT_MAJOR_VERSION GREATER 5)
    list(APPEND qtlibs_files "${QT_LIBS_PATH}/${QT_V}Qml.dll")
else()
    list(APPEND qtlibs_files "${QT_LIBS_PATH}/${QT_V}Script.dll")
endif()
if(qmlui)
    list(APPEND qtlibs_files
        "${QT_LIBS_PATH}/${QT_V}Qml.dll"
        "${QT_LIBS_PATH}/${QT_V}QmlModels.dll"
        "${QT_LIBS_PATH}/${QT_V}QmlWorkerScript.dll"
        "${QT_LIBS_PATH}/${QT_V}Quick.dll"
        "${QT_LIBS_PATH}/${QT_V}QuickControls2.dll"
        "${QT_LIBS_PATH}/${QT_V}QuickTemplates2.dll"
        "${QT_LIBS_PATH}/${QT_V}Sql.dll"
        "${QT_LIBS_PATH}/${QT_V}3DCore.dll"
        "${QT_LIBS_PATH}/${QT_V}3DExtras.dll"
        "${QT_LIBS_PATH}/${QT_V}3DInput.dll"
        "${QT_LIBS_PATH}/${QT_V}3DLogic.dll"
        "${QT_LIBS_PATH}/${QT_V}3DAnimation.dll"
        "${QT_LIBS_PATH}/${QT_V}3DQuick.dll"
        "${QT_LIBS_PATH}/${QT_V}3DQuickExtras.dll"
        "${QT_LIBS_PATH}/${QT_V}3DQuickInput.dll"
        "${QT_LIBS_PATH}/${QT_V}3DQuickAnimation.dll"
        "${QT_LIBS_PATH}/${QT_V}3DQuickRender.dll"
        "${QT_LIBS_PATH}/${QT_V}3DRender.dll"
        "${QT_LIBS_PATH}/${QT_V}Concurrent.dll"
        "${QT_LIBS_PATH}/${QT_V}Gamepad.dll"
        "${QT_LIBS_PATH}/${QT_V}PrintSupport.dll"
        "${QT_LIBS_PATH}/${QT_V}MultimediaQuick.dll"
    )
endif()
install(FILES ${qtlibs_files} DESTINATION ${qtlibs_path})

# Qt platform plugin
set(qtplatform_path "${INSTALLROOT}/${LIBSDIR}/platforms")
set(qtplatform_files "${QT_PLUGINS_PATH}/platforms/qwindows.dll")
install(FILES ${qtplatform_files} DESTINATION ${qtplatform_path})

# Qt styles plugin
set(qtstyles_path "${INSTALLROOT}/${LIBSDIR}/styles")
if(QT_MAJOR_VERSION GREATER 5)
    set(qtstyles_files "${QT_PLUGINS_PATH}/styles/qmodernwindowsstyle.dll")
else()
    set(qtstyles_files "${QT_PLUGINS_PATH}/styles/qwindowsvistastyle.dll")
endif()
install(FILES ${qtstyles_files} DESTINATION ${qtstyles_path})

if(QT_MAJOR_VERSION GREATER 5)
    set(qtmedia_path "${INSTALLROOT}/${LIBSDIR}/multimedia")
    set(qtmedia_files "${QT_PLUGINS_PATH}/multimedia/ffmpegmediaplugin.dll")
    install(FILES ${qtmedia_files} DESTINATION ${qtmedia_path})
else()
    set(qtaudio_path "${INSTALLROOT}/${LIBSDIR}/audio")
    set(qtaudio_files "${QT_PLUGINS_PATH}/audio/qtaudio_windows.dll")
    install(FILES ${qtaudio_files} DESTINATION ${qtaudio_path})

    set(qtmedia_path "${INSTALLROOT}/${LIBSDIR}/mediaservice")
    set(qtmedia_files "${QT_PLUGINS_PATH}/mediaservice/dsengine.dll"
                        "${QT_PLUGINS_PATH}/mediaservice/qtmedia_audioengine.dll")
    install(FILES ${qtmedia_files} DESTINATION ${qtmedia_path})
endif()

set(qtimageformats_path "${INSTALLROOT}/${LIBSDIR}/imageformats")
set(qtimageformats_files "${QT_PLUGINS_PATH}/imageformats/qgif.dll"
                          "${QT_PLUGINS_PATH}/imageformats/qjpeg.dll"
                          "${QT_PLUGINS_PATH}/imageformats/qsvg.dll")
install(FILES ${qtimageformats_files} DESTINATION ${qtimageformats_path})

if(qmlui)
    set(qtprintsupport_path "${INSTALLROOT}/${LIBSDIR}/printsupport")
    set(qtprintsupport_files "${QT_PLUGINS_PATH}/printsupport/windowsprintersupport.dll")
    install(FILES ${qtprintsupport_files} DESTINATION ${qtprintsupport_path})

    set(geometryloaders_path "${INSTALLROOT}/${LIBSDIR}/geometryloaders")
    set(geometryloaders_files "${QT_PLUGINS_PATH}/geometryloaders/defaultgeometryloader.dll")
    install(FILES ${geometryloaders_files} DESTINATION ${geometryloaders_path})

    set(sceneparsers_path "${INSTALLROOT}/${LIBSDIR}/sceneparsers")
    set(sceneparsers_files "${QT_PLUGINS_PATH}/sceneparsers/assimpsceneimport.dll")
    install(FILES ${sceneparsers_files} DESTINATION ${sceneparsers_path})

    set(renderers_path "${INSTALLROOT}/${LIBSDIR}/renderers")
    set(renderers_files "${QT_PLUGINS_PATH}/renderers/openglrenderer.dll")
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
                "${SYS_LIBS_PATH}/libwinpthread-1.dll"
                "${SYS_LIBS_PATH}/libicuin75.dll"
                "${SYS_LIBS_PATH}/libicuuc75.dll"
                "${SYS_LIBS_PATH}/libicudt75.dll"
                "${SYS_LIBS_PATH}/libmd4c.dll"
                "${SYS_LIBS_PATH}/libusb-1.0.dll")

if ($ENV{MSYSTEM_CARCH} STREQUAL "i686")
    list(APPEND msys_files "${SYS_LIBS_PATH}/libgcc_s_dw2-1.dll")
else()
    list(APPEND msys_files "${SYS_LIBS_PATH}/libgcc_s_seh-1.dll")
endif()

install(FILES ${msys_files} DESTINATION ${msys_path})

function(copy_system_library target libname)
	if(EXISTS ${SYS_LIBS_PATH}/${libname})
		list(APPEND mmedia_files ${SYS_LIBS_PATH}/${libname})
		set(${target} "${${target}}" PARENT_SCOPE)
    else()
        message(FATAL_ERROR "ERROR: missing DLL detected: ${libname}")
	endif()
endfunction()

# audio libraries
set(mmedia_path "${INSTALLROOT}/${LIBSDIR}")
copy_system_library(mmedia_files "libmad-0.dll")
copy_system_library(mmedia_files "libogg-0.dll")
copy_system_library(mmedia_files "libopus-0.dll")
copy_system_library(mmedia_files "libmp3lame-0.dll")
copy_system_library(mmedia_files "libmpg123-0.dll")
copy_system_library(mmedia_files "libvorbis-0.dll")
copy_system_library(mmedia_files "libvorbisenc-2.dll")
copy_system_library(mmedia_files "libFLAC.dll")
copy_system_library(mmedia_files "libsndfile-1.dll")
copy_system_library(mmedia_files "libfftw3-3.dll")
 
if(QT_MAJOR_VERSION GREATER 5)
	copy_system_library(mmedia_files "avformat-61.dll")
	copy_system_library(mmedia_files "avutil-59.dll")
	copy_system_library(mmedia_files "avcodec-61.dll")
	copy_system_library(mmedia_files "libbluray-2.dll")
	copy_system_library(mmedia_files "libgme.dll")
	copy_system_library(mmedia_files "libgnutls-30.dll")
	copy_system_library(mmedia_files "libmodplug-1.dll")
	copy_system_library(mmedia_files "librtmp-1.dll")
	copy_system_library(mmedia_files "libsrt.dll")
	copy_system_library(mmedia_files "libssh.dll")
	copy_system_library(mmedia_files "libdav1d-7.dll")
	copy_system_library(mmedia_files "libaom.dll")
	copy_system_library(mmedia_files "libcairo-2.dll")
	copy_system_library(mmedia_files "libgobject-2.0-0.dll")
	copy_system_library(mmedia_files "libgsm.dll")
	copy_system_library(mmedia_files "libjxl_threads.dll")
	copy_system_library(mmedia_files "libjxl.dll")
	copy_system_library(mmedia_files "liblzma-5.dll")
	copy_system_library(mmedia_files "liblc3-1.dll")
	copy_system_library(mmedia_files "libxml2-2.dll")
	copy_system_library(mmedia_files "libva.dll")
	copy_system_library(mmedia_files "libvpl-2.dll")
	copy_system_library(mmedia_files "libva_win32.dll")
	copy_system_library(mmedia_files "swscale-8.dll")
	copy_system_library(mmedia_files "swresample-5.dll")
	copy_system_library(mmedia_files "libopencore-amrnb-0.dll")
	copy_system_library(mmedia_files "libopencore-amrwb-0.dll")
	copy_system_library(mmedia_files "libopenjp2-7.dll")
	copy_system_library(mmedia_files "rav1e.dll")
	copy_system_library(mmedia_files "librsvg-2-2.dll")
	copy_system_library(mmedia_files "libtheoradec-1.dll")
	copy_system_library(mmedia_files "libSvtAv1Enc-2.dll")
	copy_system_library(mmedia_files "libtheoraenc-1.dll")
	copy_system_library(mmedia_files "libvpx-1.dll")
	copy_system_library(mmedia_files "libwebp-7.dll")
	copy_system_library(mmedia_files "libwebpmux-3.dll")
	copy_system_library(mmedia_files "libx265-215.dll")
	copy_system_library(mmedia_files "libx264-164.dll")
	copy_system_library(mmedia_files "xvidcore.dll")
	copy_system_library(mmedia_files "libzvbi-0.dll")
	copy_system_library(mmedia_files "libbrotlienc.dll")
	copy_system_library(mmedia_files "libgmp-10.dll")
	copy_system_library(mmedia_files "libhogweed-6.dll")
	copy_system_library(mmedia_files "libidn2-0.dll")
	copy_system_library(mmedia_files "libp11-kit-0.dll")
	copy_system_library(mmedia_files "libtasn1-6.dll")
	copy_system_library(mmedia_files "libunistring-5.dll")
	copy_system_library(mmedia_files "libnettle-8.dll")
	copy_system_library(mmedia_files "libcrypto-3-x64.dll")
	copy_system_library(mmedia_files "libfontconfig-1.dll")
	copy_system_library(mmedia_files "libpixman-1-0.dll")
	copy_system_library(mmedia_files "libffi-8.dll")
	copy_system_library(mmedia_files "libhwy.dll")
	copy_system_library(mmedia_files "libjxl_cms.dll")
	copy_system_library(mmedia_files "libsoxr.dll")
	copy_system_library(mmedia_files "libgdk_pixbuf-2.0-0.dll")
	copy_system_library(mmedia_files "libcairo-gobject-2.dll")
	copy_system_library(mmedia_files "libgio-2.0-0.dll")
	copy_system_library(mmedia_files "libpango-1.0-0.dll")
	copy_system_library(mmedia_files "libpangocairo-1.0-0.dll")
	copy_system_library(mmedia_files "libsharpyuv-0.dll")
	copy_system_library(mmedia_files "libexpat-1.dll")
	copy_system_library(mmedia_files "liblcms2-2.dll")
	copy_system_library(mmedia_files "libgomp-1.dll")
	copy_system_library(mmedia_files "libgmodule-2.0-0.dll")
	copy_system_library(mmedia_files "libtiff-6.dll")
	copy_system_library(mmedia_files "libfribidi-0.dll")
	copy_system_library(mmedia_files "libthai-0.dll")
	copy_system_library(mmedia_files "libpangoft2-1.0-0.dll")
	copy_system_library(mmedia_files "libpangowin32-1.0-0.dll")
	copy_system_library(mmedia_files "libdeflate.dll")
	copy_system_library(mmedia_files "libjbig-0.dll")
	copy_system_library(mmedia_files "libLerc.dll")
	copy_system_library(mmedia_files "libdatrie-1.dll")
endif()

install(FILES ${mmedia_files} DESTINATION ${mmedia_path})

# NullSoft installer files
set(nsis_path "${INSTALLROOT}/${DATADIR}")
if(qmlui)
    set(nsis_files qlcplus5${QT_V}.nsi)
else()
    set(nsis_files qlcplus4${QT_V}.nsi)
endif()
install(FILES ${nsis_files} DESTINATION ${nsis_path})

# install(FILES ../Sample.qxw DESTINATION ${INSTALLROOT}/${DATADIR})
