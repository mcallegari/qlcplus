%define version %(echo $QLCPLUS_VERSION)

Summary: Q Light Controller Plus - The free DMX lighting console
License: Apache License, Version 2.0
Name: qlcplus
Version: %{version}
BuildRequires: gcc-c++ pkg-config
BuildRequires: libusb-devel, libudev-devel
BuildRequires: alsa-lib-devel >= 1.0.23, libsndfile-devel, libmad-devel, fftw-devel >= 3.0.0
#BuildRequires: libola-devel
BuildRequires: desktop-file-utils
%if %{defined fedora}
BuildRequires: libftdi-devel
BuildRequires: qt5-qtbase-devel, qt5-qtmultimedia-devel, qt5-qtscript-devel, qt5-linguist
%endif
%if %{defined suse_version}
BuildRequires: libftdi1-devel
BuildRequires: libqt5-qtbase-devel, libqt5-qtmultimedia-devel, libqt5-qtscript-devel, libqt5-linguist
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
./translate.sh ui

qmake-qt5
make %{?_smp_mflags}

#############################################################################
# Install
#############################################################################

%install
INSTALL_ROOT=$RPM_BUILD_ROOT make install

desktop-file-validate %{buildroot}/%{_datadir}/applications/*.desktop

#############################################################################
# Files
#############################################################################

%files
%{_bindir}/*
%{_libdir}/libqlcplusengine.so*
%{_libdir}/libqlcplusui.so*
%{_libdir}/libqlcpluswebaccess.so*
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
%{_datadir}/qlcplus/web
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
%_mandir/*/*
%doc /usr/share/qlcplus/documents
/usr/lib/udev/rules.d/z65-anyma-udmx.rules
/usr/lib/udev/rules.d/z65-dmxusb.rules
/usr/lib/udev/rules.d/z65-fx5-hid.rules
/usr/lib/udev/rules.d/z65-peperoni.rules
/usr/lib/udev/rules.d/z65-spi.rules
