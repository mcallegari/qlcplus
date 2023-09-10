# Build QLCPlus using CMake

## Build on Windows using MSYS2

### Prepare the build system (MSYS2)

Please refer to https://github.com/mcallegari/qlcplus/wiki/Windows-Build-Qt5#prepare-the-build-system-msys2

In addition, we need to install `cmake` by running the following command in MSYS2 shell.

```bash
pacman -S mingw32/mingw-w64-i686-cmake
```

### Acquire the QLC+ sources

Please refer to https://github.com/mcallegari/qlcplus/wiki/Windows-Build-Qt5#acquire-the-qlc-sources

### Build QLC+

Close the MSYS2 shell, and from now on we will use `ming32.exe` in the installed `MSYS64` directory.
First, we need to create `build` directory and change directory into it to split source codes and binary object files.

```bash
cd <qlcplus>
mkdir build
cd build
```

We can use GCC or Ninja to compile our project

#### Using GCC

```bash
cmake -G "Unix Makefiles" ..
make
```

#### Using Ninja

```bash
cmake -G Ninja ..
ninja
```

We can generally use `cmake --build .` instead of `ninja` or `mingw32-make`.

### Build QLC+ 5

If you want to build `QLC+ 5` using `qmlui`, you can simply add `-Dqmlui=ON` parameter in the CMake command like the following.

```bash
cmake -G "Unix Makefiles" -Dqmlui=ON ..
make
```

## Build on Windows using CMake, MinGW and QT

This method is not recommended since it can not find the 3rd party libraries such as `libusb` or `libfftw3`

### Prepare the build system (CMake, Qt)

First, we need to install CMake for Windows.
We can download CMake from https://cmake.org/download/.

Next, we need to install QT creator with MinGW on Windows.
We recommend to download QT from the official website https://www.qt.io/download and check `MinGW` compilers when we install it.

### Building

After we download the source code of QLCPlus we can build it using embedded MinGW in Qt.
We can compile QLC+ in 64bit mode using the 64bit MinGW compiler.

```cmd
cd build
cmake -DCMAKE_PREFIX_PATH="C:/Qt/Qt5.12.12/5.12.12/mingw73_64/lib/cmake" -G"MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER="C:/Qt/Qt5.12.12/Tools/mingw730_64/bin/g++.exe" -DCMAKE_C_COMPILER="C:/Qt/Qt5.12.12/Tools/mingw730_64/bin/gcc.exe" -DCMAKE_MAKE_PROGRAM="C:/Qt/Qt5.12.12/Tools/mingw730_64/bin/mingw32-make.exe" ..
"C:/Qt/Qt5.12.12/Tools/mingw730_64/bin/mingw32-make.exe"
```

And we can also compile QLC+ in 32bit mode using the 32bit MinGW compiler.

```cmd
cmake -DCMAKE_PREFIX_PATH="C:/Qt/Qt5.12.12/5.12.12/mingw73_32/lib/cmake" -G"MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER="C:/Qt/Qt5.12.12/Tools/mingw730_32/bin/c++.exe" -DCMAKE_C_COMPILER="C:/Qt/Qt5.12.12/Tools/mingw730_32/bin/gcc.exe" -DCMAKE_MAKE_PROGRAM="C:/Qt/Qt5.12.12/Tools/mingw730_32/bin/mingw32-make.exe" ..
"C:/Qt/Qt5.12.12/Tools/mingw730_32/bin/mingw32-make.exe"
```

Compiling QLC+5 is similar to the section above, we just need to add `-Dqmlui=ON` parameter in the CMake command.

## Build on Linux

### Prepare the build system (Qt)

We can use either of the official Qt packages or Debian Qt packages.

#### <b>Install the official Qt packages</b>

If you want to use official Qt for building QLC, you can install it from the [official link](https://www.qt.io/download-qt-installer-oss).

You can select several versions of QT to install.

Qt will be installed in the `Qt` directory in the user `home` directory i.e. `~/Qt` by default.

#### <b>Install the Debian Qt packages</b>

If you want to use the Debian Qt packages instead of the official Qt packages, you will need to install it by running the following command in Ubuntu.

```bash
sudo apt install -y qtcreator qtbase5-dev qt5-qmake libqt5svg5-dev qt3d5-dev qtdeclarative5-dev qttools5-dev qt3d-defaultgeometryloader-plugin qt3d-assimpsceneimport-plugin qml-module-qt3d qml-module-qtmultimedia
```

for installing Qt5 packages or

```bash
sudo apt install -y qt6-base-dev qt6-tools-dev qt6-tools-dev-tools qt6-l10n-tools qt6-multimedia-dev qt6-declarative-dev qt6-3d-dev libqt6svg6-dev libqt6serialport6-dev
```

for installing Qt6 packages.

### Build QLC+ 4 on Linux

After you download (or clone) the source code of QLC+, we also need to create `build` directory and you can simply compile it by specifying the `CMAKE_PREFIX_PATH` to the appropriate path.

#### <b>Build QLC+ 4 using the official Qt packages</b>

The `CMAKE_PREFIX_PATH` can be found in the installed Qt directory.

```bash
cd <qlcplus>/build
cmake -DCMAKE_PREFIX_PATH="/home/<user>/Qt/5.15.2/gcc_64/lib/cmake" ..
make
```

If you want to compile it using Qt6, you can just simply specify the CMake path of Qt6.

```bash
cmake -DCMAKE_PREFIX_PATH="/home/<user>/Qt/6.5.0/gcc_64/lib/cmake" ..
```

#### <b>Build QLC+ 4 using the Debian Qt packages</b>

```bash
cd <qlcplus>/build
cmake -DCMAKE_PREFIX_PATH="/usr/lib/x86_64-linux-gnu/cmake/Qt5" ..
make
```

If you want to compile it using Qt6, you can just simply replace Qt5 with Qt6 in CMake command.

```bash
cmake -DCMAKE_PREFIX_PATH="/usr/lib/x86_64-linux-gnu/cmake/Qt6" ..
```

### Create a QLC+ Debian package

Just run the following command from the build folder:
```bash
cpack -G DEB
```

### Build QLC+ 5 on Linux

Building QLC+ 5 is similar to building QLC+ 4, we can just add `-Dqmlui=ON` option to build QLC+5 with `qmlui`.

```bash
cmake -Dqmlui=ON -DCMAKE_PREFIX_PATH="/usr/lib/x86_64-linux-gnu/cmake/Qt5" ..
make
```

You can select other appropriate `CMAKE_PREFIX_PATH` as described in the section above.

### Running and installing `qlcplus`

After you build `qlcplus`, you can simply run it with `make run` command.

And you can also install it with `sudo make install` command.

But when you build `qlcplus` with the official Qt installation which is not installed in system path like `/usr/lib/`, you will fail to run the installed `qlcplus`.
In this case, you will need to specify `LD_LIBRARY_PATH` so that the `qlcplus` program can find its dependencies like `libQt5Core.so.5`.

```bash
export LD_LIBRARY_PATH=/home/<user>/Qt/5.15.2/gcc_64/lib/
qlcplus
```

## Work History (For developer notes)

### qmake2cmake

I used `qmake2cmake` project to convert Qt `.pro` files to `CMake` files.

Please refer to https://www.qt.io/blog/introducing-qmake2cmake

First of all, I downloaded `qmake2cmake` by running:

```cmd
python -m pip install qmake2cmake
```

Next in the root directory of the qlcplus source code, run the following command

```cmd
cd <qlcplus>
qmake2cmake_all.exe . --min-qt-version 5.10
```

The following files were not successfully converted (2 of 141):
".\platforms\macos\macos.pro"
".\plugins\dmxusb\src\src.pro"
So need to convert these 2 project files manually.

### Modify auto-generated `CMakeList.txt` files manually

Next, I tried to `cmake` using MSVC2017.

```bash
mkdir build
cd build
cmake -DCMAKE_PREFIX_PATH="C:\Qt\Qt5.12.12\5.12.12\msvc2017\lib\cmake" ..
```

Next, since we need a compatibility with Qt5 and Qt6, I needed to replace some directives in `CMakeList.txt` files like the following.

```text
qt_add_library => add_library
qt_add_executable => add_executable
qt_add_plugin => add_library

if(CONFIG(win32)) => if(WIN32)
if(CONFIG(coverage)) => if(coverage)
if(CONFIG(udev)) => if(udev)
if(CONFIG(iokit)) => if(iokit)
```

If you want to define a Macro like `CONFIG(udev)` in Qt \*.pro file, you can add `-Dudev=ON` as a CMake parameter
E.g.

```cmd
cmake -DCMAKE_PREFIX_PATH="C:\Qt\Qt5.12.12\5.12.12\msvc2017\lib\cmake" -Dudev=ON ..
```
