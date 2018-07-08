%define version %(echo $QLCPLUS_VERSION)

Summary: Q Light Controller Plus - The free DMX lighting console
License: Apache License, Version 2.0
Group: Other
Name: qlcplus
Prefix: /usr
Provides: qlcplus
BuildRequires: qt-devel >= 4.6, libftdi-devel >= 0.17, libusb-devel >= 0.1.12, alsa-lib-devel >= 1.0.23
Requires: qt >= 4.6
Release: 1
Source: qlcplus-%{version}.tar.gz
URL: http://www.qlcplus.org/
Buildroot: /tmp/qlcplusrpm
Version: %{version}
%description
Q Light Controller - The free Linux DMX lighting desk. Includes also fixture definitions, input profiles and plugins.

#############################################################################
# Preparation
#############################################################################

%prep
%setup -q

#############################################################################
# Build
#############################################################################

%build
qmake-qt4
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
%{_datadir}/appdata/*
%_libdir/qt4/plugins/qlcplus/audio/libmadplugin.so
%_libdir/qt4/plugins/qlcplus/audio/libsndfileplugin.so
%_libdir/qt4/plugins/qlcplus/libenttecwing.so
%_libdir/qt4/plugins/qlcplus/libhidplugin.so
%_libdir/qt4/plugins/qlcplus/libdmx4linux.so
%_libdir/qt4/plugins/qlcplus/libmidiplugin.so
%_libdir/qt4/plugins/qlcplus/libdmxusb.so
%_libdir/qt4/plugins/qlcplus/libpeperoni.so
%_libdir/qt4/plugins/qlcplus/libudmx.so
%_libdir/qt4/plugins/qlcplus/libosc.so
%_libdir/qt4/plugins/qlcplus/libartnet.so
%_libdir/qt4/plugins/qlcplus/libe131.so
%_libdir/qt4/plugins/qlcplus/libspi.so
%_libdir/qt4/plugins/qlcplus/libloopback.so
%doc /usr/share/qlcplus/documents/*
/usr/lib/udev/rules.d/z65-dmxusb.rules
/usr/lib/udev/rules.d/z65-anyma-udmx.rules
/usr/lib/udev/rules.d/z65-peperoni.rules
/usr/lib/udev/rules.d/z65-fx5-hid.rules
/usr/lib/udev/rules.d/z65-spi.rules

