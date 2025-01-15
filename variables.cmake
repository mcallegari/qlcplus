if(QT_VERSION_MAJOR GREATER 5)
    set(QT_MAJOR_VERSION 6)
else(QT_VERSION_MAJOR EQUAL 5)
    set(QT_MAJOR_VERSION 5)
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

add_definitions(-DUNICODE)

set(APPNAME "Q Light Controller Plus")
set(FXEDNAME "Fixture Definition Editor")

if(ANDROID OR IOS)
    set(qmlui ON)
endif()

if(qmlui)
    add_definitions(-DQMLUI)
    set(APPVERSION "5.0.0 GIT")
else()
    set(APPVERSION "4.14.0")
endif()

if(UNIX)
    set(OLA_GIT "/usr/src/ola") # OLA directories
endif()


if(APPLE)
    set(CMAKE_MACOSX_RPATH ON)
    set(CMAKE_SKIP_BUILD_RPATH FALSE)
    set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
    set(CMAKE_INSTALL_RPATH "@executable_path/../Frameworks")
endif()

if(WIN32)
    set(DESTDIR "./")
endif()

# if(WIN32)
#     set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--enable-auto-import")
# endif()

# Installation paths
if (WIN32)
    set(INSTALLROOT "$ENV{SystemDrive}/qlcplus")
elseif (APPLE)
    set(INSTALLROOT "$ENV{HOME}/QLC+.app/Contents")
elseif (UNIX)
    set(INSTALLROOT "/usr")
endif ()

if (ANDROID)
    set(INSTALLROOT "/")
elseif (IOS)
    set(INSTALLROOT "/")
endif ()
if (NOT ${INSTALL_ROOT} STREQUAL "/")
    set(INSTALLROOT ${INSTALL_ROOT}/${INSTALLROOT})
    message("Set INSTALL_ROOT ${INSTALL_ROOT}")
endif()

IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  SET(CMAKE_INSTALL_PREFIX ${INSTALLROOT} CACHE PATH "install prefix" FORCE)
ENDIF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

# Binaries
if (WIN32)
    set(BINDIR "")
elseif (APPLE)
    set(BINDIR "MacOS")
elseif (UNIX)
    set(BINDIR "bin")
endif ()

if (ANDROID)
    set(BINDIR "bin")
elseif (IOS)
    set(BINDIR "")
endif()

# Libraries
if (WIN32)
    set(LIBSDIR "")
elseif (APPLE)
    set(LIBSDIR "Frameworks")
elseif (UNIX)
    set(LIBSDIR "${CMAKE_INSTALL_LIBDIR}")
endif ()

if (ANDROID)
    set(LIBSDIR "/libs/armeabi-v7a")
elseif (IOS)
    set(LIBSDIR "lib")
endif ()

# Data
if (WIN32)
    set(DATADIR "")
elseif (APPLE)
    set(DATADIR "Resources")
elseif (UNIX)
    set(DATADIR "share/qlcplus")
endif ()

if (ANDROID)
    set(DATADIR "/assets")
elseif (IOS)
    set(DATADIR "")
elseif (appimage)
    set(DATADIR "../share/qlcplus")
endif ()

# User Data
if (WIN32)
    set(USERDATADIR "QLC+")
elseif (APPLE)
    set(USERDATADIR "Library/Application Support/QLC+")
elseif (UNIX)
    set(USERDATADIR ".qlcplus")
endif ()

if (ANDROID)
    set(USERDATADIR ".qlcplus")
elseif (IOS)
    set(USERDATADIR ".qlcplus")
endif ()

# Documentation
if (WIN32)
    set(DOCSDIR "Documents")
elseif (APPLE)
    set(DOCSDIR "${DATADIR}/Documents")
elseif (UNIX)
    set(DOCSDIR "${DATADIR}/documents")
endif ()

if (ANDROID)
    set(DOCSDIR "${DATADIR}/documents")
elseif (IOS)
    set(DOCSDIR "Documents")
endif ()

# Input profiles
if (WIN32)
    set(INPUTPROFILEDIR "InputProfiles")
elseif (APPLE)
    set(INPUTPROFILEDIR "${DATADIR}/InputProfiles")
elseif (UNIX)
    set(INPUTPROFILEDIR "${DATADIR}/inputprofiles")
endif ()

if (ANDROID)
    set(INPUTPROFILEDIR "${DATADIR}/inputprofiles")
elseif (IOS)
    set(INPUTPROFILEDIR "InputProfiles")
endif ()

# User input profiles
if (WIN32)
    set(USERINPUTPROFILEDIR "${USERDATADIR}/InputProfiles")
elseif (APPLE)
    set(USERINPUTPROFILEDIR "${USERDATADIR}/InputProfiles")
elseif (UNIX)
    set(USERINPUTPROFILEDIR "${USERDATADIR}/inputprofiles")
endif ()

if (ANDROID)
    set(USERINPUTPROFILEDIR "${USERDATADIR}/inputprofiles")
elseif (IOS)
    set(USERINPUTPROFILEDIR "${USERDATADIR}/InputProfiles")
endif ()

# Midi templates
if (WIN32)
    set(MIDITEMPLATEDIR "MidiTemplates")
elseif (APPLE)
    set(MIDITEMPLATEDIR "${DATADIR}/MidiTemplates")
elseif (UNIX)
    set(MIDITEMPLATEDIR "${DATADIR}/miditemplates")
endif ()

if (ANDROID)
    set(MIDITEMPLATEDIR "${DATADIR}/miditemplates")
elseif (IOS)
    set(MIDITEMPLATEDIR "MidiTemplates")
endif ()

# User midi templates
if (WIN32)
    set(USERMIDITEMPLATEDIR "${USERDATADIR}/MidiTemplates")
elseif (APPLE)
    set(USERMIDITEMPLATEDIR "${USERDATADIR}/MidiTemplates")
elseif (UNIX)
    set(USERMIDITEMPLATEDIR "${USERDATADIR}/miditemplates")
endif ()

if (ANDROID)
    set(USERMIDITEMPLATEDIR "${USERDATADIR}/miditemplates")
elseif (IOS)
    set(USERMIDITEMPLATEDIR "${USERDATADIR}/MidiTemplates")
endif ()

# Channel modifiers templates
if (WIN32)
    set(MODIFIERSTEMPLATEDIR "ModifiersTemplates")
elseif (APPLE)
    set(MODIFIERSTEMPLATEDIR "${DATADIR}/ModifiersTemplates")
else ()
    set(MODIFIERSTEMPLATEDIR "${DATADIR}/modifierstemplates")
endif ()

if (ANDROID)
    set(MODIFIERSTEMPLATEDIR "${DATADIR}/modifierstemplates")
elseif (IOS)
    set(MODIFIERSTEMPLATEDIR "ModifiersTemplates")
endif ()

# User modifiers templates
if (WIN32)
    set(USERMODIFIERSTEMPLATEDIR "${USERDATADIR}/ModifiersTemplates")
elseif (APPLE)
    set(USERMODIFIERSTEMPLATEDIR "${USERDATADIR}/ModifiersTemplates")
elseif (UNIX)
    set(USERMODIFIERSTEMPLATEDIR "${USERDATADIR}/modifierstemplates")
endif ()

if (ANDROID)
    set(USERMODIFIERSTEMPLATEDIR "${USERDATADIR}/modifierstemplates")
elseif (IOS)
    set(USERMODIFIERSTEMPLATEDIR "${USERDATADIR}/ModifiersTemplates")
endif ()

# Fixtures
if (WIN32)
    set(FIXTUREDIR "Fixtures")
elseif (APPLE)
    set(FIXTUREDIR "${DATADIR}/Fixtures")
elseif (UNIX)
    set(FIXTUREDIR "${DATADIR}/fixtures")
endif ()

if (ANDROID)
    set(FIXTUREDIR "${DATADIR}/fixtures")
elseif (IOS)
    set(FIXTUREDIR "Fixtures")
endif ()

# Gobos
if (WIN32)
    set(GOBODIR "Gobos")
elseif (APPLE)
    set(GOBODIR "${DATADIR}/Gobos")
elseif (UNIX)
    set(GOBODIR "${DATADIR}/gobos")
endif ()

if (ANDROID)
    set(GOBODIR "${DATADIR}/gobos")
elseif (IOS)
    set(GOBODIR "Gobos")
endif ()

# User fixtures
if (WIN32)
    set(USERFIXTUREDIR "${USERDATADIR}/Fixtures")
elseif (APPLE)
    set(USERFIXTUREDIR "${USERDATADIR}/Fixtures")
elseif (UNIX)
    set(USERFIXTUREDIR "${USERDATADIR}/fixtures")
endif ()

if (ANDROID)
    set(USERFIXTUREDIR "${USERDATADIR}/fixtures")
elseif (IOS)
    set(USERFIXTUREDIR "${USERDATADIR}/Fixtures")
endif ()

# Plugins
if (WIN32)
    set(PLUGINDIR "Plugins")
elseif (APPLE)
    set(PLUGINDIR "PlugIns")
elseif (UNIX)
    if (appimage)
        set(PLUGINDIR "../lib/qt${QT_MAJOR_VERSION}/plugins/qlcplus")
    else ()
        set(PLUGINDIR "${LIBSDIR}/qt${QT_MAJOR_VERSION}/plugins/qlcplus")
    endif ()
endif ()

if (ANDROID OR IOS)
    set(PLUGINDIR "Plugins")
endif ()


# Audio Plugins
if (WIN32)
    set(AUDIOPLUGINDIR "${PLUGINDIR}/Audio")
elseif (APPLE)
    set(AUDIOPLUGINDIR "${PLUGINDIR}/Audio")
elseif (UNIX)
    set(AUDIOPLUGINDIR "${PLUGINDIR}/audio")
endif ()

if (ANDROID OR IOS)
    set(AUDIOPLUGINDIR "${PLUGINDIR}/Audio")
endif ()


# Translations
if (WIN32)
    set(TRANSLATIONDIR "")
elseif (APPLE)
    set(TRANSLATIONDIR "${DATADIR}/Translations")
elseif (UNIX)
    set(TRANSLATIONDIR "${DATADIR}/translations")
endif ()

if (ANDROID)
    set(TRANSLATIONDIR "${DATADIR}/translations")
elseif (IOS)
    set(TRANSLATIONDIR "")
endif ()

# RGB Scripts
if (WIN32)
    set(RGBSCRIPTDIR "RGBScripts")
elseif (APPLE)
    set(RGBSCRIPTDIR "${DATADIR}/RGBScripts")
else ()
    set(RGBSCRIPTDIR "${DATADIR}/rgbscripts")
endif ()

if (ANDROID)
    set(RGBSCRIPTDIR "${DATADIR}/rgbscripts")
elseif (IOS)
    set(RGBSCRIPTDIR "RGBScripts")
endif ()

# User RGB Scripts
if (WIN32)
    set(USERRGBSCRIPTDIR "${USERDATADIR}/RGBScripts")
elseif (APPLE)
    set(USERRGBSCRIPTDIR "${USERDATADIR}/RGBScripts")
elseif (UNIX)
    set(USERRGBSCRIPTDIR "${USERDATADIR}/rgbscripts")
endif ()

if (ANDROID)
    set(USERRGBSCRIPTDIR "${USERDATADIR}/rgbscripts")
elseif (IOS)
    set(USERRGBSCRIPTDIR "${USERDATADIR}/RGBScripts")
endif ()

# Web Files
if (WIN32)
    set(WEBFILESDIR "Web")
elseif (APPLE)
    set(WEBFILESDIR "${DATADIR}/Web")
else ()
    set(WEBFILESDIR "${DATADIR}/web")
endif ()

if (ANDROID)
    set(WEBFILESDIR "${DATADIR}/web")
elseif (IOS)
    set(WEBFILESDIR "Web")
endif ()

# Samples
if (WIN32)
    set(SAMPLESDIR "${INSTALLROOT}")
else ()
    set(SAMPLESDIR "${INSTALLROOT}/${DATADIR}")
endif ()

# 3D Meshes
if (WIN32)
    set(MESHESDIR "Meshes")
elseif (APPLE)
    set(MESHESDIR "${DATADIR}/Meshes")
else ()
    set(MESHESDIR "${DATADIR}/meshes")
endif ()

if (ANDROID)
    set(MESHESDIR "${DATADIR}/meshes")
elseif (IOS)
    set(MESHESDIR "Meshes")
endif ()


# Color filters
if (WIN32)
    set(COLORFILTERSDIR "ColorFilters")
elseif (APPLE)
    set(COLORFILTERSDIR "${DATADIR}/ColorFilters")
else ()
    set(COLORFILTERSDIR "${DATADIR}/colorfilters")
endif ()

if (ANDROID)
    set(COLORFILTERSDIR "${DATADIR}/colorfilters")
elseif (IOS)
    set(COLORFILTERSDIR "ColorFilters")
endif ()

# User Color filters
if (WIN32)
    set(USERCOLORFILTERSDIR "${USERDATADIR}/ColorFilters")
elseif (APPLE)
    set(USERCOLORFILTERSDIR "${USERDATADIR}/ColorFilters")
elseif (UNIX)
    set(USERCOLORFILTERSDIR "${USERDATADIR}/colorfilters")
endif ()

if (ANDROID)
    set(USERCOLORFILTERSDIR "${USERDATADIR}/colorfilters")
elseif (IOS)
    set(USERCOLORFILTERSDIR "${USERDATADIR}/ColorFilters")
endif ()

# udev rules
if(UNIX AND NOT APPLE)
    if (${INSTALL_ROOT} STREQUAL "/")
        set(UDEVRULESDIR "/etc/udev/rules.d")
    else()
        set(UDEVRULESDIR "${INSTALL_ROOT}/etc/udev/rules.d")
    endif()
endif()

# AppStream metadata
if(UNIX AND NOT APPLE)
    set(METAINFODIR "${INSTALLROOT}/share/metainfo")
endif()

# man
if(UNIX AND NOT APPLE)
    set(MANDIR "share/man/man1/")
endif()

# Get QT_QMAKE_EXECUTABLE path
# Get the length of the CMAKE_PREFIX_PATH variable
string(LENGTH "${CMAKE_PREFIX_PATH}" CMAKE_PREFIX_PATH_LENGTH)

# Check if the CMAKE_PREFIX_PATH ends with a slash
if (CMAKE_PREFIX_PATH_LENGTH GREATER 0)
    # Subtract one from the length to get the last index
    math (EXPR CMAKE_PREFIX_PATH_LAST_INDEX "${CMAKE_PREFIX_PATH_LENGTH}-1")
    string(SUBSTRING "${CMAKE_PREFIX_PATH}" ${CMAKE_PREFIX_PATH_LAST_INDEX} 1 CMAKE_PREFIX_PATH_LAST_CHAR)
    if ((${CMAKE_PREFIX_PATH_LAST_CHAR} STREQUAL "/") OR (${CMAKE_PREFIX_PATH_LAST_CHAR} STREQUAL "\\"))
        set(QT_QMAKE_EXECUTABLE "${CMAKE_PREFIX_PATH}../../bin/qmake")
    else()
        set(QT_QMAKE_EXECUTABLE "${CMAKE_PREFIX_PATH}/../../bin/qmake")
    endif()
endif()

# Get QT_INSTALL_PREFIX, QT_INSTALL_LIBS, QT_INSTALL_PLUGINS variables
execute_process(
    COMMAND ${QT_QMAKE_EXECUTABLE} -query QT_INSTALL_PREFIX
    OUTPUT_VARIABLE QT_INSTALL_PREFIX
)

execute_process(
    COMMAND ${QT_QMAKE_EXECUTABLE} -query QT_INSTALL_LIBS
    OUTPUT_VARIABLE QT_INSTALL_LIBS
)

execute_process(
    COMMAND ${QT_QMAKE_EXECUTABLE} -query QT_INSTALL_PLUGINS
    OUTPUT_VARIABLE QT_INSTALL_PLUGINS
)

if (UNIX AND NOT APPLE)
    set(QTPREFIX "${QT_INSTALL_PREFIX}")
    string(FIND "${QTPREFIX}" "/usr" inUsr)

    if (${inUsr} EQUAL 0)
        set(QTLIBSDIR "${QT_INSTALL_LIBS}")
        set(QTPLUGINSDIR "${QT_INSTALL_PLUGINS}")
        string(REPLACE "/usr/" "" LIBSDIR "${QTLIBSDIR}")
        string(REPLACE "/usr/" "" PLUGINDIR "${QTPLUGINSDIR}/qlcplus")
        set(AUDIOPLUGINDIR "${PLUGINDIR}/audio")
    endif()

    # message("QT_INSTALL_PREFIX: ${QT_INSTALL_PREFIX}")
    # message("QT_INSTALL_LIBS: ${QT_INSTALL_LIBS}")
    # message("Linux LIBSDIR:  ${INSTALLROOT}/${LIBSDIR}")
    # message("Linux PLUGINDIR: ${INSTALLROOT}/${PLUGINDIR}")
endif()

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4701")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4101")

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4456")   # Suppress warning C4456: declaration of '_container_' hides previous local declaration in foreach

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX")

elseif(NOT APPLE AND NOT IOS)

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
endif()

