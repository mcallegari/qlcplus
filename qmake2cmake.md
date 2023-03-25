First of all download qmake2cmake by running
python -m pip install qmake2cmake
Please refer to https://www.qt.io/blog/introducing-qmake2cmake

qmake2cmake_all.exe . --min-qt-version 5.10

The following files were not successfully converted (2 of 141):
    ".\platforms\macos\macos.pro"
    ".\plugins\dmxusb\src\src.pro"


mkdir build
cd build
cmake -DCMAKE_PREFIX_PATH="C:\Qt\Qt5.12.12\5.12.12\msvc2017_64\lib\cmake" ..