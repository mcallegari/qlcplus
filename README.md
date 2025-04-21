<p align="center">
  <a href="https://www.qlcplus.org/">
    <img src="resources/icons/png/qlcplus.png" alt="QLC+ Logo" height="60" />
  </a>
</p>

<h1 align="center">Q Light Controller+</h1>
<p align="center"><em>(Often abbreviated as "QLC+")</em></p>
<p align="center">
  <strong>Open-source lighting control for DMX, Artnet, s.ACN and more.</strong><br/>
  Designed for live shows, theatre, architectual installations, and venues.
</p>

<p align="center">
  <a href="https://github.com/mcallegari/qlcplus/releases/latest">
    <img src="https://img.shields.io/github/v/release/mcallegari/qlcplus" alt="Latest release version badge" /></a>
  <a href="https://github.com/mcallegari/qlcplus/releases/latest">
    <img src="https://img.shields.io/github/release-date/mcallegari/qlcplus" alt="Release date badge" /></a>
  <a href="https://github.com/mcallegari/qlcplus/commits/master/">
    <img src="https://img.shields.io/github/commits-since/mcallegari/qlcplus/latest/master" alt="Commits since latest release badge" /></a>
  <a href="https://github.com/mcallegari/qlcplus/commits/master/">
    <img src="https://img.shields.io/github/commit-activity/w/mcallegari/qlcplus" alt="Weekly commit activity badge" /></a>
  <a href="https://github.com/mcallegari/qlcplus/actions">
    <img src="https://github.com/mcallegari/qlcplus/actions/workflows/build.yml/badge.svg" alt="Build status badge" /></a>
  <a href="https://coveralls.io/github/mcallegari/qlcplus?branch=master">
    <img src="https://coveralls.io/repos/github/mcallegari/qlcplus/badge.svg?branch=master" alt="Test coverage badge" /></a>
</p>

---

<p align="center">
  <a href="https://www.qlcplus.org/download">
    <img src="https://custom-icon-badges.demolab.com/badge/-Download_QLC+-blue?style=for-the-badge&logo=download&logoColor=white" alt="Download QLC+ badge" /></a>
  <a href="https://qlcplus.org/discover/raspberry-pi">
    <img src="https://custom-icon-badges.demolab.com/badge/-Raspberry_Pi-red?style=for-the-badge&logo=cpu&logoColor=white" alt="Raspberry Pi badge" /></a>
  <a href="https://merch.qlcplus.org">
    <img src="https://custom-icon-badges.demolab.com/badge/-Store-green?style=for-the-badge&logo=home&logoColor=white" alt="Official store badge" /></a>
</p>

## Introduction

**QLC+** is powerful and user-friendly software to control lighting. QLC+ supports a [huge amount of hardware,](https://qlcplus.org/discover/compatibility) runs on Linux, Windows (10+), macOS (10.12+), and Raspberry Pi. Whether you're an experienced lighting professional or just getting started, QLC+ empowers you to take control of your lighting fixtures with ease. The primary goal of this project is to bring QLC+ to the level of available commercial software.

### Supported protocols

[![MIDI](https://img.shields.io/badge/MIDI-%23323330.svg?style=for-the-badge&logo=midi&logoColor=%23F7DF1E)](https://docs.qlcplus.org/v4/plugins/midi)
[![OSC](https://img.shields.io/badge/OSC-%23323330.svg?style=for-the-badge&logo=aiohttp&logoColor=%23F7DF1E)](https://docs.qlcplus.org/v4/plugins/osc)
[![HID](https://img.shields.io/badge/HID-%23323330.svg?style=for-the-badge&logo=applearcade&logoColor=%23F7DF1E)](https://docs.qlcplus.org/v4/plugins/hid)
[![DMX](https://img.shields.io/badge/DMX-%23323330.svg?style=for-the-badge&logo=amazonec2&logoColor=%23F7DF1E)](https://docs.qlcplus.org/v4/plugins/dmx-usb)
[![ArtNet](https://img.shields.io/badge/ArtNet-%23323330.svg?style=for-the-badge&logo=aiohttp&logoColor=%23F7DF1E)](https://docs.qlcplus.org/v4/plugins/art-net)
[![E1.31/S.ACN](https://img.shields.io/badge/E1.31%20S.ACN-%23323330.svg?style=for-the-badge&logo=aiohttp&logoColor=%23F7DF1E)](https://docs.qlcplus.org/v4/plugins/e1-31-sacn)
[![OS2L](https://img.shields.io/badge/OS2L-%23323330.svg?style=for-the-badge&logo=aiohttp&logoColor=%23F7DF1E)](https://docs.qlcplus.org/v4/plugins/os2l)

### QLC+ on social media

[![Instagram](https://img.shields.io/badge/Instagram-%23E4405F.svg?style=flat-square&logo=Instagram)](https://www.instagram.com/qlcplus/) 
[![YouTube](https://img.shields.io/badge/YouTube-%23FF0000.svg?style=flat-square&logo=YouTube)](https://www.youtube.com/watch?v=I9bccwcYQpM&list=PLHT-wIriuitDiW4A9oKSDr__Z_jcmMVdi) 
[![Facebook](https://img.shields.io/badge/Facebook-%231877F2.svg?style=flat-square&logo=Facebook)](https://www.facebook.com/qlcplus)

## Support & bug reports

We have a dedicated page to help you find support, please checkout [SUPPORT.md](SUPPORT.md). To learn about a specific feature of QLC+, take a look at the [official documentation](https://docs.qlcplus.org/). To give feedback, submit new fixtures and get new ideas, go to the [forum](https://www.qlcplus.org/forum/index.php)

### Help wanted
Click the badge below to see the currently confirmed issues with QLC+. Perhaps you can find a solution?

[![Help Wanted](https://img.shields.io/github/issues/mcallegari/qlcplus/issue%20confirmed?logo=github&color=red)](https://github.com/mcallegari/qlcplus/issues?q=is%3Aopen+is%3Aissue+label%3A%22issue+confirmed%22)


## Building QLC+

Compilation guides and platform-specific instructions are available in our [GitHub Wiki](https://github.com/mcallegari/qlcplus/wiki).

#### Developers at work

If you're regularly updating QLC+ sources with git pull, you may encounter compiler warnings, errors, or unresolved symbols. We strive to keep the `master` branch free of critical errors; however, dependencies between objects can sometimes cause issues, requiring a full package recompilation rather than just updating recent changes.

## Contributing
### Software development

We welcome contributions from the community to help make QLC+ even better. If you're working on something major, start a thread in the [Development Forum](https://www.qlcplus.org/forum/viewforum.php?f=12) first. Make sure you read the [CONTRIBUTING.md](CONTRIBUTING.md) document for more.

### Financially

If you're reading this we already appreciate you. If you're just getting started with lighting you have absolutely no obligation to give us money. When QLC+ opens up revenue opportunities for you, we'd be very thankful for your support. GitHub sponsors is the prefered option.

<img src="https://img.shields.io/github/sponsors/mcallegari" alt="GitHub Sponsors"> <a href="https://github.com/sponsors/mcallegari"><img src="https://img.shields.io/badge/sponsor-30363D?logo=GitHub-Sponsors&logoColor=#white" /></a>

If you're interested, QLC+ also has an [official store](https://merch.qlcplus.org) where you can purchase [clothing](https://qlcplus-merch.myshopify.com/collections/clothing), [themes](https://qlcplus-merch.myshopify.com/collections/themes), the [Raspberry Pi image](https://qlcplus-merch.myshopify.com/products/qlc-raspberry-pi-image) or [one-on-one consultation](https://qlcplus-merch.myshopify.com/collections/training-and-support) with an expert. 



## Thank you!

QLC+ owes its success to the dedication and expertise of numerous individuals who have generously contributed their time and skills. The following list recognizes those whose remarkable contributions have played a pivotal role in building QLC+.

![GitHub contributors](https://img.shields.io/github/contributors/mcallegari/qlcplus)

<details>
<summary>QLC+ 5</summary>
    
*   Eric Arnebäck (3D preview features)
*   Santiago Benejam Torres (Catalan translation)
*   Luis García Tornel (Spanish translation)
*   Nils Van Zuijlen, Jérôme Lebleu (French translation)
*   Felix Edelmann, Florian Edelmann (fixture definitions, German translation)
*   Jannis Achstetter (German translation)
*   Dai Suetake (Japanese translation)
*   Hannes Bossuyt (Dutch translation)
*   Aleksandr Gusarov (Russian translation)
*   Vadim Syniuhin (Ukrainian translation)
*   Mateusz Kędzierski (Polish translation)

</details>

<details>
<summary>QLC+ 4</summary>

*   Jano Svitok (bugfix, new features and improvements)
*   David Garyga (bugfix, new features and improvements)
*   Lukas Jähn (bugfix, new features)
*   Robert Box (fixtures review)
*   Thomas Achtner (ENTTEC wing improvements)
*   Joep Admiraal (MIDI SysEx init messages, Dutch translation)
*   Florian Euchner (FX5 USB DMX support)
*   Stefan Riemens (new features)
*   Bartosz Grabias (new features)
*   Simon Newton, Peter Newman (OLA plugin)
*   Janosch Frank (webaccess improvements)
*   Karri Kaksonen (DMX USB Eurolite USB DMX512 Pro support)
*   Stefan Krupop (HID DMXControl Projects e.V. Nodle U1 support)
*   Nathan Durnan (RGB scripts, new features)
*   Giorgio Rebecchi (new features)
*   Florian Edelmann (code cleanup, German translation)
*   Heiko Fanieng, Jannis Achstetter (German translation)
*   NiKoyes, Jérôme Lebleu, Olivier Humbert, Nils Van Zuijlen (French translation)
*   Raymond Van Laake (Dutch translation)
*   Luis García Tornel (Spanish translation)
*   Jan Lachman (Czech translation)
*   Nuno Almeida, Carlos Eduardo Porto de Oliveira (Portuguese translation)
*   Santiago Benejam Torres (Catalan translation)
*   Koichiro Saito, Dai Suetake (Japanese translation)
</details>

<details>
<summary>Q Light Controller</summary>

*   Stefan Krumm (Bugfixes, new features)
*   Christian Suehs (Bugfixes, new features)
*   Christopher Staite (Bugfixes)
*   Klaus Weidenbach (Bugfixes, German translation)
*   Lutz Hillebrand (uDMX plugin)
*   Matthew Jaggard (Velleman plugin)
*   Ptit Vachon (French translation)
</details>

---

<p align="center">
<a href="https://github.com/mcallegari/qlcplus/graphs/contributors">
  <img src="https://contrib.rocks/image?repo=mcallegari/qlcplus" />
</a>
</p>

---


## License
<a href="https://github.com/mcallegari/qlcplus/blob/master/COPYING">
  <img alt="GitHub License badge" src="https://img.shields.io/github/license/mcallegari/qlcplus?style=flat-square" />
</a>

Licensed under the **Apache 2.0** License.  See [COPYING](COPYING) for details.

---
<p align="center">Copyright © Heikki Junnila, Massimo Callegari</p>
<p align="center">
  <img src="https://img.shields.io/badge/c++-%2300599C.svg?style=for-the-badge&logo=c%2B%2B&logoColor=white" alt="C++ badge" />
  <img src="https://img.shields.io/badge/Qt-%23217346.svg?style=for-the-badge&logo=Qt&logoColor=white" alt="Qt badge" />
  <img src="https://img.shields.io/badge/CMake-%23008FBA.svg?style=for-the-badge&logo=cmake&logoColor=white" alt="CMake badge" />
  <img src="https://img.shields.io/badge/javascript-%23323330.svg?style=for-the-badge&logo=javascript&logoColor=%23F7DF1E" alt="JavaScript badge" />
</p>
