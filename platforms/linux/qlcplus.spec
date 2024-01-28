%define version %(echo $QLCPLUS_VERSION)
#define ui qmlui

Summary: Q Light Controller Plus - The free DMX lighting console
License: Apache License, Version 2.0
Name: qlcplus
Version: %{version}
BuildRequires:  desktop-file-utils
BuildRequires:  fdupes
BuildRequires:  gcc-c++
BuildRequires:  pkgconfig
BuildRequires:  pkgconfig(Qt5Multimedia)
BuildRequires:  pkgconfig(Qt5Script)
BuildRequires:  pkgconfig(Qt5Widgets)
BuildRequires:  pkgconfig(Qt5SerialPort)
BuildRequires:  pkgconfig(alsa)
BuildRequires:  pkgconfig(fftw3)
BuildRequires:  pkgconfig(libftdi1)
BuildRequires:  pkgconfig(libola)
BuildRequires:  pkgconfig(libudev)
BuildRequires:  pkgconfig(mad)
BuildRequires:  pkgconfig(sndfile)
%if %{defined fedora}
BuildRequires:  pkgconfig(libusb-1.0)
BuildRequires:  qt5-linguist
BuildRequires:  qt5-qtconfiguration-devel
%if "%{ui}" == "qmlui"
BuildRequires:  qt5-qt3d-devel
BuildRequires:  qt5-qtsvg-devel
%endif
%else
BuildRequires:  pkgconfig(libusb1)
BuildRequires:  libqt5-linguist-devel
BuildRequires:  update-desktop-files
%endif
Release: 1
Source: qlcplus-%{version}.tar.gz
URL: https://www.qlcplus.org/

%description
QLC+ is a fork of the great QLC project written
by Heikki Junnila. This project aims to continue
the development of QLC and to introduce new features.
The primary goal is to bring QLC+ at the level
of other lighting control commercial softwares.

#############################################################################
# Preparation
#############################################################################

%prep
%setup -q

sed -ie '/UDEVRULESDIR/s|/etc/udev/rules.d|/usr/lib/udev/rules.d|' variables.pri

#############################################################################
# Build
#############################################################################

%build
# qmake-qt5 will only include existing files in install_translations - create the .qm files first

%if "%{ui}" == "qmlui"
    ./translate.sh qmlui
    qmake-qt5 CONFIG+=qmlui
%else
    ./translate.sh ui
    qmake-qt5
%endif
make %{?_smp_mflags}

#############################################################################
# Install
#############################################################################

%install
INSTALL_ROOT=$RPM_BUILD_ROOT make install
%if "%{ui}" == "qmlui"
mv %{buildroot}/%{_bindir}/qlcplus-qml %{buildroot}/%{_bindir}/qlcplus
sed -i -e 's/Exec=qlcplus --open %f/Exec=qlcplus/g' %{buildroot}/%{_datadir}/applications/qlcplus.desktop
%endif

desktop-file-validate %{buildroot}/%{_datadir}/applications/*.desktop

#############################################################################
# Post
#############################################################################

%if %{defined suse_version}
%post -p /sbin/ldconfig
%postun -p /sbin/ldconfig
%endif

#############################################################################
# Files
#############################################################################

%files
%{_bindir}/*
%{_libdir}/libqlcplusengine.so*
%if "%{ui}" != "qmlui"
%{_libdir}/libqlcplusui.so*
%{_libdir}/libqlcpluswebaccess.so*
%endif
%dir %{_datadir}/qlcplus
%{_datadir}/applications/*
%{_datadir}/metainfo/*
%{_datadir}/mime/packages/qlcplus.xml
%{_datadir}/pixmaps/*
%{_datadir}/qlcplus/Sample.qxw
%{_datadir}/qlcplus/fixtures
%{_datadir}/qlcplus/gobos
%{_datadir}/qlcplus/inputprofiles
%{_datadir}/qlcplus/miditemplates
%{_datadir}/qlcplus/modifierstemplates
%{_datadir}/qlcplus/rgbscripts
%{_datadir}/qlcplus/translations
%if "%{ui}" == "qmlui"
%{_datadir}/qlcplus/colorfilters
%{_datadir}/qlcplus/meshes
%else
%{_datadir}/qlcplus/web
%endif
%_libdir/qt5/plugins/qlcplus/audio/libmadplugin.so
%_libdir/qt5/plugins/qlcplus/audio/libsndfileplugin.so
%_libdir/qt5/plugins/qlcplus/libartnet.so
%_libdir/qt5/plugins/qlcplus/libdmx4linux.so
%_libdir/qt5/plugins/qlcplus/libdmxusb.so
%_libdir/qt5/plugins/qlcplus/libe131.so
%_libdir/qt5/plugins/qlcplus/libenttecwing.so
%_libdir/qt5/plugins/qlcplus/libhidplugin.so
%_libdir/qt5/plugins/qlcplus/libloopback.so
%_libdir/qt5/plugins/qlcplus/libmidiplugin.so
%_libdir/qt5/plugins/qlcplus/libos2l.so
%_libdir/qt5/plugins/qlcplus/libosc.so
%_libdir/qt5/plugins/qlcplus/libpeperoni.so
%_libdir/qt5/plugins/qlcplus/libspi.so
%_libdir/qt5/plugins/qlcplus/libudmx.so
%if "%{ui}" != "qmlui"
%_mandir/*/*
%doc /usr/share/qlcplus/documents
%endif
/usr/lib/udev/rules.d/z65-anyma-udmx.rules
/usr/lib/udev/rules.d/z65-dmxusb.rules
/usr/lib/udev/rules.d/z65-fx5-hid.rules
/usr/lib/udev/rules.d/z65-peperoni.rules
/usr/lib/udev/rules.d/z65-spi.rules
