Q Light Controller Plus 4
=========================

![QLC+ LOGO](resources/icons/png/qlcplus.png)

Copyright (c) Heikki Junnila
	      Massimo Callegari

QLC+ homepage: http://www.qlcplus.org/

QLC+ on GitHub: https://github.com/mcallegari/qlcplus

DEVELOPERS AT WORK
------------------

If you're compiling QLC+ from sources and you regularly do "git pull" 
to get the latest sources, you probably end up seeing some
compiler warnings and errors from time to time. Since the whole source package
is under development, you might even encounter unresolved symbols etc. that
halt the compiler immediately. If such a thing occurs, you should do a "make
distclean" on qlcplus (top-most source directory) and then "qmake" and "make"
again. We attempt to keep the GIT master free of fatal errors and it should
compile all the time. However, some inter-object dependencies do get mixed up
sometimes and you need to compile the whole package instead of just the latest
changes. Sometimes even that doesn't work, because QLC+ installs its common
libraries to system directories, where (at least unixes) fetch them instead
of the source directory. In those cases, you might try going to the libs
directory, compile it with "make" and install with "make install" and then
attempt to re-compile the whole package with "make".

Apache 2.0 License
------------------

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

Requirements - Linux
--------------------

* Qt >= 4.6 development libraries & tools
* libudev-dev, libmad0-dev, libsndfile1-dev, libfftw3-dev

* DMX USB plugin: libftdi-dev, pkg-config
* HID plugin: No additional requirements
* MIDI plugin: libasound, libasound-dev, pkg-config
* ENTTEC Wing plugin: No additional requirements
* OLA plugin: libola, ola-dev, pkg-config (see libs/olaout/README)
* uDMX plugin: libusb, libusb-dev, pkg-config
* Peperoni plugin: libusb, libusb-dev, pkg-config
* Velleman plugin: Not available for Linux
* OSC plugin: No additional requirements
* ArtNet plugin: No additional requirements
* E1.31 plugin: No additional requirements
* Loopback plugin: No additional requirements

Requirements - Windows
----------------------

* MSYS2 environment (https://msys2.github.io/)
* Qt >= 5.0.x (http://download.qt.io/official_releases/qt/)

* DMX USB plugin: D2XX driver & development package (http://www.ftdichip.com/Drivers/D2XX.htm)
* HID plugin: No additional requirements
* MIDI plugin: No additional requirements
* ENTTEC Wing plugin: D2XX driver & development package (http://www.ftdichip.com/Drivers/D2XX.htm)
* OLA plugin: Not available
* uDMX plugin: No additional requirements
* Peperoni plugin: No additional requirements
* Velleman plugin: K8062 SDK from www.velleman.eu
* OSC plugin: No additional requirements
* ArtNet plugin: No additional requirements
* E1.31 plugin: No additional requirements
* Loopback plugin: No additional requirements

Requirements - Mac OS X
-----------------------

* XCode (http://developer.apple.com/technologies/tools/xcode.html)
* Qt SDK >= 5.0.x (http://download.qt.io/official_releases/qt/)
* macports (https://www.macports.org/)

* DMX USB plugin: macports, libftdi-dev, pkg-config
* HID plugin: No additional requirements
* MIDI plugin: No additional requirements
* ENTTEC Wing plugin: No additional requirements
* OLA plugin: libola, ola-dev, pkg-config (see libs/olaout/README)
* uDMX plugin: macports, libusb-compat, pkg-config
* Peperoni plugin: macports, libusb-compat, pkg-config
* Velleman plugin: Not available
* OSC plugin: No additional requirements
* ArtNet plugin: No additional requirements
* E1.31 plugin: No additional requirements
* Loopback plugin: No additional requirements

Compiling & Installation
------------------------

Please refer to the online wiki pages: https://github.com/mcallegari/qlcplus/wiki

Support & Bug Reports
---------------------

For discussions, feedbacks, ideas and new fixtures, go to:
http://www.qlcplus.org/forum/index.php

For developers wiki and code patches, go to:
https://github.com/mcallegari/qlcplus

Contributors
------------

### QLC+:

* Jano Svitok (bugfix, new features and improvements)
* David Garyga (bugfix, new features and improvements)
* Lukas Jähn (bugfix, new features)
* Robert Box (fixtures review)
* Thomas Achtner (ENTTEC wing improvements)
* Joep Admiraal (MIDI SysEx init messages, Dutch translation)
* Florian Euchner (FX5 USB DMX support)
* Stefan Riemens (new features)
* Bartosz Grabias (new features)
* Simon Newton, Peter Newman (OLA plugin)
* Janosch Frank (webaccess improvements)
* Karri Kaksonen (DMX USB Eurolite USB DMX512 Pro support)
* Stefan Krupop (HID DMXControl Projects e.V. Nodle U1 support)
* Nathan Durnan (RGB scripts, new features)
* Giorgio Rebecchi (new features)
* Heiko Fanieng, Jannis Achstetter (German translation)
* NiKoyes, Jérôme Lebleu, Olivier Humbert (French translation)
* Raymond Van Laake (Dutch translation)
* Luis García Tornel (Spanish translation)
* Jan Lachman (Czech translation)
* Nuno Almeida, Carlos Eduardo Porto de Oliveira (Portuguese translation)
* Santiago Benejam Torres (Catalan translation)
* Koichiro Saito (Japanese translation)

### QLC:

* Stefan Krumm (Bugfixes, new features)
* Christian Suehs (Bugfixes, new features)
* Christopher Staite (Bugfixes)
* Klaus Weidenbach (Bugfixes, German translation)
* Lutz Hillebrand (uDMX plugin)
* Matthew Jaggard (Velleman plugin)
* Ptit Vachon (French translation)

