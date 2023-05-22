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
    set(APPVERSION "5.0.0 Beta 3")
else()
    set(APPVERSION "4.12.7 GIT")
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

# if(WIN32)
#     set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--enable-auto-import")
# endif()

# Installation paths
if(WIN32)
    set(INSTALLROOT "$ENV{SystemDrive}/qlcplus")
elseif(APPLE)
    set(INSTALLROOT "~/QLC+.app/Contents")
else()
    set(INSTALLROOT "/usr")
endif()

IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  SET(CMAKE_INSTALL_PREFIX ${INSTALLROOT} CACHE PATH "install prefix" FORCE)
ENDIF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

# Binaries
if(WIN32)
    set(BINDIR "")
else()
    set(BINDIR "bin")
endif()

# Libraries
if(WIN32)
    set(LIBSDIR "")
elseif(APPLE)
    set(LIBSDIR "Frameworks")
else()
    set(LIBSDIR "lib/x86_64-linux-gnu")
endif()

# Data
if(WIN32)
    set(DATADIR "")
else()
    set(DATADIR "share/qlcplus")
endif()

# User Data
if(WIN32)
    set(USERDATADIR "QLC+")
elseif(APPLE)
    set(USERDATADIR "Library/Application Support/QLC+")
else()
    set(USERDATADIR ".qlcplus")
endif()

# Documentation
if(WIN32)
    set(DOCSDIR "Documents")
else()
    set(DOCSDIR "${DATADIR}/documents")
endif()

# Input profiles
if(WIN32)
    set(INPUTPROFILEDIR "InputProfiles")
else()
    set(INPUTPROFILEDIR "${DATADIR}/inputprofiles")
endif()

# User input profiles
if(WIN32)
    set(USERINPUTPROFILEDIR "${USERDATADIR}/InputProfiles")
else()
    set(USERINPUTPROFILEDIR "${USERDATADIR}/inputprofiles")
endif()

# Midi templates
if(WIN32)
    set(MIDITEMPLATEDIR "MidiTemplates")
else()
    set(MIDITEMPLATEDIR "${DATADIR}/miditemplates")
endif()

# User midi templates
if(WIN32)
    set(USERMIDITEMPLATEDIR "${USERDATADIR}/MidiTemplates")
else()
    set(USERMIDITEMPLATEDIR "${USERDATADIR}/miditemplates")
endif()

# Channel modifiers templates
if(WIN32)
    set(MODIFIERSTEMPLATEDIR "ModifiersTemplates")
else()
    set(MODIFIERSTEMPLATEDIR "${DATADIR}/modifierstemplates")
endif()

# User midi templates
if(WIN32)
    set(USERMODIFIERSTEMPLATEDIR "${USERDATADIR}/ModifiersTemplates")
else()
    set(USERMODIFIERSTEMPLATEDIR "${USERDATADIR}/modifierstemplates")
endif()

# Fixtures
if(WIN32)
    set(FIXTUREDIR "Fixtures")
else()
    set(FIXTUREDIR "${DATADIR}/fixtures")
endif()

# Gobos
if(WIN32)
    set(GOBODIR "Gobos")
else()
    set(GOBODIR "${DATADIR}/gobos")
endif()

# User fixtures
if(WIN32)
    set(USERFIXTUREDIR "${USERDATADIR}/Fixtures")
else()
    set(USERFIXTUREDIR "${USERDATADIR}/fixtures")
endif()

# Plugins
if(WIN32)
    set(PLUGINDIR "Plugins")
else()
    set(PLUGINDIR "${LIBSDIR}/qt5/plugins/qlcplus")
endif()

# Audio Plugins
set(AUDIOPLUGINDIR "${PLUGINDIR}/Audio")

# Translations
if(WIN32)
    set(TRANSLATIONDIR "")
else()
    set(TRANSLATIONDIR "${DATADIR}/translations")
endif()

# RGB Scripts
if(WIN32)
    set(RGBSCRIPTDIR "RGBScripts")
else()
    set(RGBSCRIPTDIR "${DATADIR}/rgbscripts")
endif()

# User RGB Scripts
if(WIN32)
    set(USERRGBSCRIPTDIR "${USERDATADIR}/RGBScripts")
else()
    set(USERRGBSCRIPTDIR "${USERDATADIR}/rgbscripts")
endif()

# Web Files
if(WIN32)
    set(WEBFILESDIR "Web")
else()
    set(WEBFILESDIR "${DATADIR}/web")
endif()

# Samples
set(SAMPLESDIR "${INSTALLROOT}/${DATADIR}")

# 3D Meshes
if(WIN32)
    set(MESHESDIR "Meshes")
elseif(APPLE)
    set(MESHESDIR "${DATADIR}/Meshes")
elseif(ANDROID)
    set(MESHESDIR "${DATADIR}/meshes")
elseif(IOS)
    set(MESHESDIR "Meshes")
else()
    set(MESHESDIR "${DATADIR}/meshes")
endif()

# Color filters
if(WIN32)
    set(COLORFILTERSDIR "ColorFilters")
elseif(APPLE)
    set(COLORFILTERSDIR "${DATADIR}/ColorFilters")
elseif(ANDROID)
    set(COLORFILTERSDIR "${DATADIR}/colorfilters")
elseif(IOS)
    set(COLORFILTERSDIR "ColorFilters")
else()
    set(COLORFILTERSDIR "${DATADIR}/colorfilters")
endif()

# User Color filters
if(WIN32)
    set(USERCOLORFILTERSDIR "${USERDATADIR}/ColorFilters")
elseif(APPLE)
    set(USERCOLORFILTERSDIR "${USERDATADIR}/ColorFilters")
elseif(ANDROID)
    set(USERCOLORFILTERSDIR "${USERDATADIR}/colorfilters")
elseif(IOS)
    set(USERCOLORFILTERSDIR "${USERDATADIR}/ColorFilters")
else()
    set(USERCOLORFILTERSDIR "${USERDATADIR}/colorfilters")
endif()

# udev rules
if(UNIX AND NOT APPLE)
    set(UDEVRULESDIR "/etc/udev/rules.d")
endif()

# AppStream metadata
if(UNIX AND NOT APPLE)
    set(METAINFODIR "${INSTALLROOT}/share/metainfo")
endif()

# man
if(UNIX AND NOT APPLE)
    set(MANDIR "share/man/man1/")
endif()

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4701")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4101")
    
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4456")   # Suppress warning C4456: declaration of '_container_' hides previous local declaration in foreach

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX")
elseif(NOT APPLE AND NOT IOS)
    # Check the version of g++
    execute_process(COMMAND g++ --version OUTPUT_VARIABLE GPP_VERSION)
    if(GPP_VERSION MATCHES "4.6.[0-9]")
        #message("g++ version 4.6 found")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error=strict-overflow")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-local-typedefs")
    endif()

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
endif()

