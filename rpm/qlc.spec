%define version %(echo $QLC_VERSION)

Summary: Q Light Controller - The free DMX lighting console
License: GPLv2
Group: Other
Name: qlc
Prefix: /usr
Provides: qlc
BuildRequires: qt-devel >= 4.6, libftdi-devel >= 0.17, libusb-devel >= 0.1.12, alsa-lib-devel >= 1.0.23
Requires: qt >= 4.6
Release: 1
Source: qlc-%{version}.tar.gz
URL: http://qlc.sourceforge.net/
Buildroot: /tmp/qlcrpm
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
/usr/bin/qlc
/usr/lib/libqlcengine*
/usr/lib/libqlcui*
/usr/share/qlc/translations/*.qm
/usr/share/applications/qlc.desktop
/usr/share/pixmaps/qlc.png
/usr/bin/qlc-fixtureeditor
/usr/share/applications/qlc-fixtureeditor.desktop
/usr/share/pixmaps/qlc-fixtureeditor.png
/usr/share/qlc/fixtures/*
/usr/share/qlc/inputprofiles/*
/usr/share/qlc/Sample.qxw
/usr/share/mime/packages/qlc.xml
%doc /usr/share/qlc/documents/*
/usr/lib/qt4/plugins/qlc/input/libhidinput.so
/usr/lib/qt4/plugins/qlc/input/libewinginput.so
/usr/lib/qt4/plugins/qlc/input/libmidiinput.so
/usr/lib/qt4/plugins/qlc/output/libdmx4linuxout.so
/usr/lib/qt4/plugins/qlc/output/libenttecdmxusbout.so
/etc/udev/rules.d/z65-enttec-dmxusb.rules
/usr/lib/qt4/plugins/qlc/output/libudmxout.so
/etc/udev/rules.d/z65-anyma-udmx.rules
/usr/lib/qt4/plugins/qlc/output/libpeperoniout.so
/etc/udev/rules.d/z65-peperoni.rules
/usr/lib/qt4/plugins/qlc/output/libmidiout.so
