<a href="https://www.qlcplus.org/">
    <img src="resources/icons/png/qlcplus.png" alt="QLC Logo" title="qlcplus.png" align="right" height="60" />
</a>

# Q Light Controller Plus
![GitHub release)](https://img.shields.io/github/v/release/mcallegari/qlcplus)
![QLC+ Github Actions CI Build](https://github.com/mcallegari/qlcplus/actions/workflows/build.yml/badge.svg)

## Introduction
QLC+ is a powerful and user-friendly software designed for lighting control. Whether you're an experienced lighting professional or just getting started, QLC+ empowers you to take control of your lighting fixtures with ease. The primary goal of this project is to bring QLC+ to the level of available commercial software. 
QLC+ runs on Linux, Windows (7+), macOS (10.7+) and the Raspberry Pi.

Copyright © Heikki Junnila, Massimo Callegari


### Key Resources:
<table>
<thead>
</thead>
<tbody>
<tr>
<td style="text-align:right">Home Page</td>
<td style="text-align:left"><a href="https://qlcplus.org"><img src="https://img.shields.io/badge/qlcplus.org-blue?logo=grav" alt="Static Badge"></a></td>
</tr>
<tr>
<td style="text-align:right">Documentation</td>
<td style="text-align:left"><a href="https://docs.qlcplus.org"><img src="https://img.shields.io/badge/docs.qlcplus.org-blue?logo=grav" alt="Static Badge"></a></td>
</tr>
<tr>
<td style="text-align:right">Official Forum</td>
<td style="text-align:left"><a href="https://qlcplus.org/forum/"><img src="https://img.shields.io/badge/qlcplus.org/forum-grey?logo=php" alt="Static Badge"></a></td>
</tr>
<tr>
<td style="text-align:right">GitHub Sponsorship</td>
<td style="text-align:left"><img src="https://img.shields.io/github/sponsors/mcallegari" alt="GitHub Sponsors"> <a href="https://github.com/sponsors/mcallegari"><img src="https://img.shields.io/badge/sponsor-30363D?logo=GitHub-Sponsors&logoColor=#white" /></a></td>
</tr>
<tr>
<td style="text-align:right">Official Merch</td>
<td style="text-align:left"><a href="https://merch.qlcplus.org"><img src="https://img.shields.io/badge/merch.qlcplus.org-Official_Merchandice-green?logo=shopify" alt="Static Badge"></a></td>
</tr>
</tbody>
</table>

### QLC+ Social Media:

[![Instagram](https://img.shields.io/badge/Instagram-%23E4405F.svg?style=for-the-badge&logo=Instagram&logoColor=white)](https://www.instagram.com/qlcplus/) [![YouTube](https://img.shields.io/badge/YouTube-%23FF0000.svg?style=for-the-badge&logo=YouTube&logoColor=white)](https://www.youtube.com/watch?v=I9bccwcYQpM&list=PLHT-wIriuitDiW4A9oKSDr__Z_jcmMVdi) [![Facebook](https://img.shields.io/badge/Facebook-%231877F2.svg?style=for-the-badge&logo=Facebook&logoColor=white)](https://www.facebook.com/qlcplus)



## Contributing
We welcome contributions from the community to help make QLC+ even better. Before diving into coding, we encourage you to start a discussion in our [Software Development](https://www.qlcplus.org/forum/viewforum.php?f=12) forum if you're considering adding a new feature or making significant changes. This provides an opportunity for feedback, collaboration, and ensuring alignment with the project's goals.

Further guidelines are available in the [CONTRIBUTING.md](CONTRIBUTING.md) document.


### 🚧 DEVELOPERS AT WORK 🚧


If you're compiling QLC+ from sources and you regularly do "git pull"
to get the latest sources, you probably end up seeing some
compiler warnings and errors from time to time. Since the whole source package
is under development, you might even encounter unresolved symbols etc. If such a thing occurs, you should do a "make
distclean" on qlcplus (top-most source directory) and then "qmake" and "make"
again. We attempt to keep the GIT master free of fatal errors and it should
compile all the time. However, some inter-object dependencies do get mixed up
sometimes and you need to compile the whole package instead of just the latest
changes. Sometimes even that doesn't work, because QLC+ installs its common
libraries to system directories, where (at least unixes) fetch them instead
of the source directory. In those cases, you might try going to the libs
directory, compile it with "make" and install with "make install" and then
attempt to re-compile the whole package with "make".

## Compiling And Installation

Please refer to the online wiki pages: https://github.com/mcallegari/qlcplus/wiki
## Requirements
### Linux


* Qt >= 5.0 development libraries & tools
* libudev-dev, libmad0-dev, libsndfile1-dev, libfftw3-dev
* DMX USB plugin: libftdi-dev, pkg-config
* MIDI plugin: libasound, libasound-dev, pkg-config
* OLA plugin: libola, ola-dev, pkg-config (see libs/olaout/README)
* uDMX plugin: libusb, libusb-dev, pkg-config
* Peperoni plugin: libusb, libusb-dev, pkg-config
* Velleman plugin: **Not available**

### Windows

* MSYS2 environment (https://msys2.github.io/)
* DMX USB plugin: D2XX driver & development package (http://www.ftdichip.com/Drivers/D2XX.htm)
* ENTTEC Wing plugin: D2XX driver & development package (http://www.ftdichip.com/Drivers/D2XX.htm)
* OLA plugin: **Not available**
* Velleman plugin: K8062 SDK from www.velleman.eu


### Mac OS X

* XCode (http://developer.apple.com/technologies/tools/xcode.html)
* Qt >= 5.0.x (http://download.qt.io/official_releases/qt/)
* macports (https://www.macports.org/)
* DMX USB plugin: macports, libftdi-dev, pkg-config
* OLA plugin: libola, ola-dev, pkg-config (see libs/olaout/README)
* uDMX plugin: macports, libusb-compat, pkg-config
* Peperoni plugin: macports, libusb-compat, pkg-config
* Velleman plugin: **Not available**


## Support & Bug Reports

For discussions, feedbacks, ideas and new fixtures, go to:
https://www.qlcplus.org/forum/index.php

For developers wiki and code patches, go to:
https://github.com/mcallegari/qlcplus

## Contributors
QLC+ owes its success to the dedication and expertise of numerous individuals who have generously contributed their time and skills. The following list recognizes those whose remarkable contributions have played a pivotal role in shaping QLC+ into what it is today.

### QLC+ 5:

* Eric Arnebäck (3D preview features)
* Santiago Benejam Torres (Catalan translation)
* Luis García Tornel (Spanish translation)
* Nils Van Zuijlen, Jérôme Lebleu (French translation)
* Felix Edelmann, Florian Edelmann (fixture definitions, German translation)
* Jannis Achstetter (German translation)
* Dai Suetake (Japanese translation)
* Hannes Bossuyt (Dutch translation)
* Aleksandr Gusarov (Russian translation)
* Vadim Syniuhin (Ukrainian translation)
* Mateusz Kędzierski (Polish translation)

### QLC+ 4:

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
* Florian Edelmann (code cleanup, German translation)
* Heiko Fanieng, Jannis Achstetter (German translation)
* NiKoyes, Jérôme Lebleu, Olivier Humbert, Nils Van Zuijlen (French translation)
* Raymond Van Laake (Dutch translation)
* Luis García Tornel (Spanish translation)
* Jan Lachman (Czech translation)
* Nuno Almeida, Carlos Eduardo Porto de Oliveira (Portuguese translation)
* Santiago Benejam Torres (Catalan translation)
* Koichiro Saito, Dai Suetake (Japanese translation)

### QLC:

* Stefan Krumm (Bugfixes, new features)
* Christian Suehs (Bugfixes, new features)
* Christopher Staite (Bugfixes)
* Klaus Weidenbach (Bugfixes, German translation)
* Lutz Hillebrand (uDMX plugin)
* Matthew Jaggard (Velleman plugin)
* Ptit Vachon (French translation)

## Apache 2.0
![GitHub License](https://img.shields.io/github/license/mcallegari/qlcplus)

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.

----
![C++](https://img.shields.io/badge/c++-%2300599C.svg?style=for-the-badge&logo=c%2B%2B&logoColor=white) ![Qt](https://img.shields.io/badge/Qt-%23217346.svg?style=for-the-badge&logo=Qt&logoColor=white) ![CMake](https://img.shields.io/badge/CMake-%23008FBA.svg?style=for-the-badge&logo=cmake&logoColor=white) ![JavaScript](https://img.shields.io/badge/javascript-%23323330.svg?style=for-the-badge&logo=javascript&logoColor=%23F7DF1E)
