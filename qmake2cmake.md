First of all download qmake2cmake by running
python -m pip install qmake2cmake
Please refer to https://www.qt.io/blog/introducing-qmake2cmake

qmake2cmake_all.exe . --min-qt-version 5.10

The following files were not successfully converted (2 of 141):
    ".\platforms\macos\macos.pro"
    ".\plugins\dmxusb\src\src.pro"


mkdir build
cd build
cmake -DCMAKE_PREFIX_PATH="C:\Qt\Qt5.12.12\5.12.12\msvc2017\lib\cmake" ..

Modify CMakeList.txt files
qt_add_library => add_library
qt_add_executable => add_executable

if(CONFIG(win32)) => if(WIN32)
if(CONFIG(coverage)) => if(coverage)
if(CONFIG(udev)) => if(udev)
if(CONFIG(iokit)) => if(iokit)

If you want to define a Macro like CONFIG(udev) in Qt *.pro file, you can add -Dudev=ON as a CMake parameter
E.g.
cmake -DCMAKE_PREFIX_PATH="C:\Qt\Qt5.12.12\5.12.12\msvc2017\lib\cmake" -Dudev=ON ..