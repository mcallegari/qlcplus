#!/bin/bash
#
# Script to create a self contained AppImage
# Requires wget and chrpath
# Export QTDIR before running this, like:
#   export QTDIR=/home/user/Qt5.9.4/5.9.4/gcc_64

# Exit on error
set -e

TARGET_DIR=~/qlcplus.AppDir

# Build
if [ -n "$QTDIR" ]; then
    $QTDIR/bin/qmake CONFIG+=appimage CONFIG+=qmlui
else
    qmake CONFIG+=appimage CONFIG+=qmlui
fi

make -j8
make check

mkdir $TARGET_DIR
make INSTALL_ROOT=$TARGET_DIR install

strip $TARGET_DIR/usr/bin/qlcplus-qml
strip $TARGET_DIR/usr/lib/libqlcplusengine.so.1.0.0

chrpath -r "../lib" $TARGET_DIR/usr/bin/qlcplus-qml

pushd $TARGET_DIR/usr/bin
find . -name plugins.qmltypes -type f -delete
find . -name *.qmlc -type f -delete
rm -rf QtQuick/Extras QtQuick/Particles.2 QtQuick/XmlListModel
rm -rf QtQuick/Controls.2/designer QtQuick/Controls.2/Material
rm -rf QtQuick/Controls.2/Universal QtQuick/Controls.2/Fusion
rm -rf QtQuick/Controls.2/Imagine QtQuick/Controls.2/Scene2D
popd

if [ ! -f $TARGET_DIR/AppRun]; then
    wget -c https://github.com/AppImage/AppImageKit/releases/download/continuous/AppRun-x86_64 -O $TARGET_DIR/AppRun
    chmod a+x $TARGET_DIR/AppRun
fi

cp resources/icons/svg/qlcplus.svg $TARGET_DIR
cp platforms/linux/qlcplus.desktop $TARGET_DIR
sed -i -e 's/Exec=qlcplus --open %f/Exec=qlcplus-qml/g' $TARGET_DIR/qlcplus.desktop

if [ ! -f /tmp/appimagetool-x86_64.AppImage ]; then
    wget -c https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage -O /tmp/appimagetool-x86_64.AppImage
    chmod a+x /tmp/appimagetool-x86_64.AppImage
fi

pushd $TARGET_DIR/..
/tmp/appimagetool-x86_64.AppImage $TARGET_DIR
popd

make clean

