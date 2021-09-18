#!/bin/bash
#
# Script to create a self contained AppImage
# Requires wget and chrpath
# Export QTDIR before running this, like:
#   export QTDIR=/home/user/Qt5.9.4/5.9.4/gcc_64

# Exit on error
set -e

TARGET_DIR=~/qlcplus.AppDir

# Compile translations
./translate.sh "qmlui"

# Build
if [ -n "$QTDIR" ]; then
    $QTDIR/bin/qmake CONFIG+=appimage CONFIG+=qmlui
else
    qmake CONFIG+=appimage CONFIG+=qmlui
fi

NUM_CPUS=`nproc` || true
if [ -z "$NUM_CPUS" ]; then
    NUM_CPUS=8
fi

make -j$NUM_CPUS
make check

if [ ! -d "$TARGET_DIR" ]; then
    mkdir $TARGET_DIR
fi
make INSTALL_ROOT=$TARGET_DIR install

strip $TARGET_DIR/usr/bin/qlcplus-qml
# see variables.pri, where to find the LIBSDIR
find $TARGET_DIR/usr/lib/ -name libqlcplusengine.so.1.0.0 -exec strip -v {} \;

# FIXME: no rpath or runpath tag found.
chrpath -r "../lib" $TARGET_DIR/usr/bin/qlcplus-qml || true

pushd $TARGET_DIR/usr/bin
find . -name plugins.qmltypes -type f -delete
find . -name *.qmlc -type f -delete
rm -rf QtQuick/Extras QtQuick/Particles.2 QtQuick/XmlListModel
rm -rf QtQuick/Controls.2/designer QtQuick/Controls.2/Material
rm -rf QtQuick/Controls.2/Universal QtQuick/Controls.2/Fusion
rm -rf QtQuick/Controls.2/Imagine QtQuick/Controls.2/Scene2D
popd

# There might be a new version of the tool available.
wget -c https://github.com/AppImage/AppImageKit/releases/download/continuous/AppRun-x86_64 -O $TARGET_DIR/AppRun
chmod a+x $TARGET_DIR/AppRun

cp -v resources/icons/svg/qlcplus.svg $TARGET_DIR
cp -v platforms/linux/qlcplus.desktop $TARGET_DIR
sed -i -e 's/Exec=qlcplus --open %f/Exec=qlcplus-qml/g' $TARGET_DIR/qlcplus.desktop

# There might be a new version of the tool available.
wget -c https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage -O /tmp/appimagetool-x86_64.AppImage
chmod a+x /tmp/appimagetool-x86_64.AppImage

pushd $TARGET_DIR/..
/tmp/appimagetool-x86_64.AppImage -v $TARGET_DIR
popd

echo "The application is now available at ~/Q_Light_Controller_Plus-x86_64.AppImage"
