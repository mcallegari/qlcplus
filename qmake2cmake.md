# Build QLCPlus using CMake

## Build on Windows using MSYS2
### Prepare the build system (MSYS2)
Please refer to https://github.com/mcallegari/qlcplus/wiki/Windows-Build-Qt5#prepare-the-build-system-msys2

In addition, we need to install `cmake` and `mingw32-make` by running the following command in MSYS2 shell.
```
pacman -S mingw32/mingw-w64-i686-cmake mingw32/mingw-w64-i686-make
```
### Acquire the QLC+ sources
Please refer to https://github.com/mcallegari/qlcplus/wiki/Windows-Build-Qt5#acquire-the-qlc-sources

### Build QLC++
Close the MSYS2 shell, and from now on we will use `ming32.exe` in the installed `MSYS64` directory.
First, we need to create `build` directory and change directory into it to split source codes and binary object files.
```
cd <qlcplus>
mkdir build
cd build
```
We can use Ninja or MinGW to compile our project
#### Using Ninja
```
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ..
ninja
```
#### Using MinGW
```
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release ..
mingw32-make
```
We can generally use `cmake --build .` instead of `ninja` or `mingw32-make`.

### Build QLC++ 5
If you want to build `QLC++ 5` using `qmlui`, you can simply add `-Dqmlui=ON` parameter in the CMake command like the following.
```
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -Dqmlui=ON ..
mingw32-make
```

## Build on Windows using CMake, MinGW and QT.
This method is not recommended since it can not find the 3rd party libraries such as `libusb` or `libfftw3`
### Preparation
First, we need to install CMake for Windows.
We can download CMake from https://cmake.org/download/.

Next, we need to install QT creator with MinGW on Windows.
We recommend to download QT from the official website https://www.qt.io/download and check `MinGW` compilers when we install it.
### Building
After we download the source code of QLCPlus we can build it using embedded MinGW in Qt.
We can compile QLC++ in 64bit mode using the 64bit MinGW compiler.
```
cd build
cmake -DCMAKE_PREFIX_PATH="C:/Qt/Qt5.12.12/5.12.12/mingw73_64/lib/cmake" -G"MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER="C:/Qt/Qt5.12.12/Tools/mingw730_64/bin/g++.exe" -DCMAKE_C_COMPILER="C:/Qt/Qt5.12.12/Tools/mingw730_64/bin/gcc.exe" -DCMAKE_MAKE_PROGRAM="C:/Qt/Qt5.12.12/Tools/mingw730_64/bin/mingw32-make.exe" ..
"C:/Qt/Qt5.12.12/Tools/mingw730_64/bin/mingw32-make.exe"
```
And we can also compile QLC++ in 32bit mode using the 32bit MinGW compiler.
```
cmake -DCMAKE_PREFIX_PATH="C:/Qt/Qt5.12.12/5.12.12/mingw73_32/lib/cmake" -G"MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER="C:/Qt/Qt5.12.12/Tools/mingw730_32/bin/c++.exe" -DCMAKE_C_COMPILER="C:/Qt/Qt5.12.12/Tools/mingw730_32/bin/gcc.exe" -DCMAKE_MAKE_PROGRAM="C:/Qt/Qt5.12.12/Tools/mingw730_32/bin/mingw32-make.exe" ..
"C:/Qt/Qt5.12.12/Tools/mingw730_32/bin/mingw32-make.exe"
```
Compiling QLC++5 is similar to the section above, we just need to add `-Dqmlui=ON` parameter in the CMake command.

## Build on Linux
### Preparation
If you want to use Qt6, you will need to install it by running the following command in Ubuntu.
```
sudo apt-get install qt6* libqt6svg6-dev
```
### Build QLC++ 4 on Linux
After you download (or clone) the source code of QLC++, we also need to create `build` directory and you can simply compile it by running the following command.
```
cd <qlcplus>/build
cmake -Dudev=ON -DCMAKE_PREFIX_PATH="/usr/lib/x86_64-linux-gnu/cmake/Qt5" ..
make
```
If you want to compile it using Qt6, you can just simply replace Qt5 with Qt6 in CMake command.
```
cmake -Dudev=ON -DCMAKE_PREFIX_PATH="/usr/lib/x86_64-linux-gnu/cmake/Qt6" ..
```
### Build QLC++ 5 on Linux 
Same with above, add `-Dqmlui=ON` option to build QLC++5 with `qmlui`.
```
cmake -Dqmlui=ON -Dudev=ON -DCMAKE_PREFIX_PATH="/usr/lib/x86_64-linux-gnu/cmake/Qt5" ..
make
```
# Work History (For developer notes)
## qmake2cmake
I used `qmake2cmake` project to convert Qt `.pro` files to `CMake` files.

Please refer to https://www.qt.io/blog/introducing-qmake2cmake

First of all, I downloaded `qmake2cmake` by running:
```bash
python -m pip install qmake2cmake
```
Next in the root directory of the qlcplus source code, run the following command
```
cd <qlcplus>
qmake2cmake_all.exe . --min-qt-version 5.10
```

The following files were not successfully converted (2 of 141):
    ".\platforms\macos\macos.pro"
    ".\plugins\dmxusb\src\src.pro"
So need to convert these 2 project files manually.

Next, I tried to `cmake` using MSVC2017.
```
mkdir build
cd build
cmake -DCMAKE_PREFIX_PATH="C:\Qt\Qt5.12.12\5.12.12\msvc2017\lib\cmake" ..
```

Next, since we need a compatibility with Qt5 and Qt6, I needed to replace some directives in `CMakeList.txt` files like the following.

```
qt_add_library => add_library
qt_add_executable => add_executable
qt_add_plugin => add_library

if(CONFIG(win32)) => if(WIN32)
if(CONFIG(coverage)) => if(coverage)
if(CONFIG(udev)) => if(udev)
if(CONFIG(iokit)) => if(iokit)
```

If you want to define a Macro like `CONFIG(udev)` in Qt *.pro file, you can add `-Dudev=ON` as a CMake parameter
E.g.
```
cmake -DCMAKE_PREFIX_PATH="C:\Qt\Qt5.12.12\5.12.12\msvc2017\lib\cmake" -Dudev=ON ..
```