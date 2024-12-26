#!/bin/bash
#
# Script to create a self contained AppImage using CMake
# Requires wget and chrpath
# If you want to use the official Qt packages, please export QTDIR before running this, like:
#   export QTDIR=/home/user/Qt/5.15.2/gcc_64
# Or you can use the system Qt libraries instead by not specifying QTDIR.

# Exit on error
set -e

TARGET_DIR=$HOME/qlcplus.AppDir
CMAKE_OPTS=""

if [ "$1" == "qmlui" ]; then
    ./translate.sh "qmlui"
    CMAKE_OPTS="-Dqmlui=ON"
else
    ./translate.sh "ui"
fi

# Build
if [ -d build ]; then
    rm -rf build
fi
mkdir build
cd build

if [ -n "$QTDIR" ]; then
    cmake -DCMAKE_PREFIX_PATH="$QTDIR/lib/cmake/" $CMAKE_OPTS -Dappimage=ON -DINSTALL_ROOT=$TARGET_DIR ..
else
    cmake -DCMAKE_PREFIX_PATH="/usr/lib/x86_64-linux-gnu/cmake/Qt5" $CMAKE_OPTS -Dappimage=ON -DINSTALL_ROOT=$TARGET_DIR ..
fi

NUM_CPUS=$(nproc) || true
if [ -z "$NUM_CPUS" ]; then
    NUM_CPUS=8
fi

make -j$NUM_CPUS
make check

if [ ! -d "$TARGET_DIR" ]; then
    mkdir $TARGET_DIR
fi
make install

find $TARGET_DIR/usr/lib/ -name 'libqlcplusengine.so*' -exec strip -v {} \;

if [ "$1" == "qmlui" ]; then
    strip $TARGET_DIR/usr/bin/qlcplus-qml
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
    sed -i -e 's/Exec=qlcplus --open %f/Exec=qlcplus-qml/g' $TARGET_DIR/qlcplus.desktop
else
    strip $TARGET_DIR/usr/bin/qlcplus
    chrpath -r "../lib" $TARGET_DIR/usr/bin/qlcplus || true
    sed -i -e 's/Exec=qlcplus --open %f/Exec=qlcplus/g' $TARGET_DIR/qlcplus.desktop
fi

cp -v ../resources/icons/svg/qlcplus.svg $TARGET_DIR
cp -v ../platforms/linux/qlcplus.desktop $TARGET_DIR

# There might be a new version of the tool available.
wget -c https://github.com/AppImage/AppImageKit/releases/download/continuous/AppRun-x86_64 -O $TARGET_DIR/AppRun
chmod a+x $TARGET_DIR/AppRun

# There might be a new version of the tool available.
wget -c https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage -O /tmp/appimagetool-x86_64.AppImage
chmod a+x /tmp/appimagetool-x86_64.AppImage

pushd $TARGET_DIR/..
/tmp/appimagetool-x86_64.AppImage -v $TARGET_DIR
popd

echo "The application is now available at ~/Q_Light_Controller_Plus-x86_64.AppImage"
