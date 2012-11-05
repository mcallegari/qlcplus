Q Light Controller 3

Copyright (c) Heikki Junnila

EMail: hjunnila@users.sf.net
QLC at SourceForge: http://qlc.sf.net

DEVELOPERS AT WORK
------------------

If you're compiling QLC from sources and you regularly do "svn update" (or
something similar) to get the latest sources, you probably end up seeing some
compiler warnings and errors from time to time. Since the whole source package
is under development, you might even encounter unresolved symbols etc. that
halt the compiler immediately. If such a thing occurs, you should do a "make
distclean" in trunk (top-most source directory) and then "qmake" and "make"
again. We attempt to keep the SVN head free of fatal errors and it should
compile all the time. However, some inter-object dependencies do get mixed up
sometimes and you need to compile the whole package instead of just the latest
changes. Sometimes even that doesn't work, because QLC installs its common
libraries to system directories, where (at least unixes) fetch them instead
of the source directory. In those cases, you might try going to the libs
directory, compile it with "make" and install with "make install" and then
attempt to re-compile the whole package with "make".

License
-------
This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
Version 2 as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details. The license is
in the file "COPYING".

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.


Requirements - Linux
--------------------
* Qt >= 4.6 development libraries & tools

* Enttec DMX USB Output plugin: libftdi-dev, pkg-config
* HID Input plugin: No additional requirements
* MIDI Input plugin: libasound, libasound-dev, pkg-config
* ENTTEC Wing Input plugin: No additional requirements
* MIDI Output plugin: libasound, libasound-dev, pkg-config
* OLA Output plugin: libola, ola-dev, pkg-config (see libs/olaout/README)
* uDMX Output plugin: libusb, libusb-dev, pkg-config
* Peperoni Output plugin: libusb, libusb-dev, pkg-config
* Velleman Output: Not available for Linux

Requirements - Windows
----------------------
* MinGW environment (ftp://ftp.qt.nokia.com/misc/MinGW-3.4.2.exe)
* Qt >= 4.6 (http://qt.nokia.com/downloads/windows-cpp)

* Enttec DMX USB Output plugin: No additional requirements
* HID Input plugin: Not available
* MIDI Input plugin: No additional requirements
* ENTTEC Wing Input plugin: D2XX driver & development package (http://www.ftdichip.com/Drivers/D2XX.htm)
* MIDI Output plugin: No additional requirements
* LLA Output plugin: Not available for Windows
* uDMX Output plugin: No additional requirements
* Peperoni Output plugin: No additional requirements
* Velleman Output: K8062 SDK from www.velleman.eu

Requirements - Mac OS X
-----------------------
* XCode (http://developer.apple.com/technologies/tools/xcode.html)
* Qt SDK >= 4.6 (http://qt.nokia.com/downloads/qt-for-open-source-cpp-development-on-mac-os-x)

* Enttec DMX USB Output plugin: macports, libftdi-dev, pkg-config
* HID Input plugin: Not available
* MIDI Input plugin: No additional requirements
* ENTTEC Wing Input plugin: No additional requirements
* MIDI Output plugin: No additional requirements
* OLA Output plugin: libola, ola-dev, pkg-config (see libs/olaout/README)
* uDMX Output plugin: macports, libusb-compat, pkg-config
* Peperoni Output plugin: macports, libusb-compat, pkg-config
* Velleman Output: Not available for OSX


Compiling & Installation
------------------------

For windows, just install the latest Qt package from Trolltech along with the
default MinGW setup and start the Qt command prompt. For Linux or OSX, in
addition to the Qt libraries, you need also Qt development packages.

As normal user, type to your X terminal:
	qmake
and then:
	make
and then (as administrator/root):
	make install


Support & Bug Reports
---------------------
In general, support requests and bug reports should be sent to 
qlc-devel@lists.sf.net.

For Wiki, bugs and other related information, go to
http://sourceforge.net/apps/trac/qlc/ (->View Tickets)


Contributors
------------
Stefan Krumm <stefankrumm@users.sf.net> (Bugfixes, new features)
Christian Suehs <dance-or-die@users.sf.net> (Bugfixes, new features)
Christopher Staite <chris_staite@users.sf.net> (Bugfixes)
Klaus Weidenbach <dawnbreak@users.sf.net> (Bugfixes, German translation)
Simon Newton <newtons@iinet.net> (OLA plugin)
Lutz Hillebrand <lutz.h@hagen.de> (uDMX plugin)
Matthew Jaggard <matthew@jaggard.org.uk> (Velleman plugin)
Ptit Vachon <6ril@no-log.org> (French translation)
NiKoyes <nikoyes@free.fr> (French translation)
