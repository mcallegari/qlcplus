%define version %(echo $QLCPLUS_VERSION)

Summary: Q Light Controller Plus - The free DMX lighting console
License: Apache License, Version 2.0
Group: Other
Name: qlcplus
Version: %{version}
Prefix: /usr
Provides: qlcplus
BuildRequires: gcc-c++ pkg-config
BuildRequires: qt5-qtbase-devel, qt5-qttranslations, qt5-qtconfiguration-devel
BuildRequires: qt5-qtmultimedia-devel, qt5-qtscript-devel, alsa-lib, qt5-linguist
BuildRequires: desktop-file-utils, libusb-devel, libftdi-devel, alsa-lib-devel >= 1.0.23
BuildRequires: libudev-devel, fftw3-devel
#BuildRequires: libola-devel
BuildRequires: libsndfile-devel, libmad-devel, dos2unix
Requires: qt5-qtbase, qt5-qtscript, qt5-qtmultimedia
Release: 1
Source: qlcplus-%{version}.tar.gz
URL: https://www.qlcplus.org/
Buildroot: /tmp/qlcplusrpm

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

#############################################################################
# Build
#############################################################################

%build
qmake-qt5
make

#############################################################################
# Install
#############################################################################

%install
rm -rf $RPM_BUILD_ROOT
INSTALL_ROOT=$RPM_BUILD_ROOT make install

#############################################################################
# Clean
#############################################################################

#%clean
#rm -rf $RPM_BUILD_ROOT

#############################################################################
# Files
#############################################################################

%files
%defattr(-,root,root)
%{_bindir}/*
%{_libdir}/libqlcplusengine.so.*
%{_libdir}/libqlcplusui.so.*
%{_libdir}/libqlcpluswebaccess.so.*
%{_datadir}/qlcplus/translations/*
%{_datadir}/applications/*
%{_datadir}/pixmaps/*
%{_datadir}/qlcplus/fixtures/*
%{_datadir}/qlcplus/inputprofiles/*
%{_datadir}/qlcplus/rgbscripts/*
%{_datadir}/qlcplus/Sample.qxw
%{_datadir}/qlcplus/gobos/*
%{_datadir}/qlcplus/miditemplates/*
%{_datadir}/qlcplus/modifierstemplates/*
%{_datadir}/qlcplus/web/*
%{_datadir}/mime/packages/qlcplus.xml
%{_datadir}/metainfo/*
%_libdir/qt5/plugins/qlcplus/audio/libmadplugin.so
%_libdir/qt5/plugins/qlcplus/audio/libsndfileplugin.so
%_libdir/qt5/plugins/qlcplus/libenttecwing.so
%_libdir/qt5/plugins/qlcplus/libhidplugin.so
%_libdir/qt5/plugins/qlcplus/libdmx4linux.so
%_libdir/qt5/plugins/qlcplus/libmidiplugin.so
%_libdir/qt5/plugins/qlcplus/libdmxusb.so
%_libdir/qt5/plugins/qlcplus/libpeperoni.so
%_libdir/qt5/plugins/qlcplus/libudmx.so
%_libdir/qt5/plugins/qlcplus/libos2l.so
%_libdir/qt5/plugins/qlcplus/libosc.so
%_libdir/qt5/plugins/qlcplus/libartnet.so
%_libdir/qt5/plugins/qlcplus/libe131.so
%_libdir/qt5/plugins/qlcplus/libspi.so
%_libdir/qt5/plugins/qlcplus/libloopback.so
%doc /usr/share/qlcplus/documents/*
/usr/lib/udev/rules.d/z65-dmxusb.rules
/usr/lib/udev/rules.d/z65-anyma-udmx.rules
/usr/lib/udev/rules.d/z65-peperoni.rules
/usr/lib/udev/rules.d/z65-fx5-hid.rules
/usr/lib/udev/rules.d/z65-spi.rules

