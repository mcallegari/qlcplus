#!/bin/bash

APP_DIR=/c/qlcplus
ROOT_DIR=$PWD

# cleanup previous builds
rm -rf $APP_DIR
rm -rf build
mkdir build
cd build

# Build
# export QTDIR=/c/projects/Qt/6.9.0/mingw_64/
if [ -n "$QTDIR" ]; then
 if [ "$1" == "qmlui" ]; then
    cmake -DCMAKE_PREFIX_PATH="$QTDIR/lib/cmake" -Dqmlui=on ..
 else
    cmake -DCMAKE_PREFIX_PATH="$QTDIR/lib/cmake" ..
 fi
else
    echo "QTDIR not set. Aborting."
    exit 1
fi

ninja

if [ ! $? -eq 0 ]; then
    echo Compiler error. Aborting package creation.
    exit $?
fi

# Install to target
ninja install
if [ ! $? -eq 0 ]; then
    echo Installation error. Aborting package creation.
    exit $?
fi

cd ..

echo "Run windeployqt..."
cd $APP_DIR
if [ "$1" == "qmlui" ]; then
  $QTDIR/bin/windeployqt --qmldir $ROOT_DIR/qmlui/qml qlcplusengine.dll Plugins/dmxusb.dll qlcplus-qml.exe
  rm sceneparsers/gltfsceneexport.dll
else
  $QTDIR/bin/windeployqt qlcplusengine.dll qlcplusui.dll qlcpluswebaccess.dll Plugins/dmxusb.dll qlcplus.exe
fi

# remove uneeded stuff
rm -rf generic networkinformation qmltooling renderplugins tls translations


# Create Installer
makensis -X'SetCompressor /FINAL lzma' qlcplus*.nsi
 
