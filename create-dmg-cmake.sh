#!/bin/bash
#VERSION=$(head -1 debian/changelog | sed 's/.*(\(.*\)).*/\1/')
VERSION=$(grep -m 1 APPVERSION variables.pri | cut -d '=' -f 2 | sed -e 's/^[[:space:]]*//' | tr ' ' _ | tr -d '\r\n')
APP_DIR=~/QLC+.app
BIN_DIR=$APP_DIR/Contents/MacOS

if [ "$1" == "qmlui" ]; then
    OPTS="-Dqmlui=on"
fi

# cleanup previous builds
rm -rf $APP_DIR
rm *.dmg
rm rw.*

rm -rf build
mkdir build
cd build

if [ ! -n "$SIGNATURE" ]; then
    echo "This build WILL NOT be signed. Please export SIGNATURE to sign it."
fi

# Build
if [ -n "$QTDIR" ]; then
    CMAKE_OSX_DEPLOYMENT_TARGET=12.0
    [ -d "$QTDIR/lib/cmake/Qt5Core" ] && CMAKE_OSX_DEPLOYMENT_TARGET=10.13
    cmake -DCMAKE_PREFIX_PATH="$QTDIR/lib/cmake" -DCMAKE_OSX_DEPLOYMENT_TARGET=$CMAKE_OSX_DEPLOYMENT_TARGET $OPTS ..
else
    echo "QTDIR not set. Aborting."
    exit 1
fi

NUM_CPUS=`sysctl -n hw.ncpu` || true
if [ -z "$NUM_CPUS" ]; then
    NUM_CPUS=4
fi

make -j$NUM_CPUS

if [ ! $? -eq 0 ]; then
    echo Compiler error. Aborting package creation.
    exit $?
fi

# Install to ~/QLC+.app/
make install/fast
if [ ! $? -eq 0 ]; then
    echo Installation error. Aborting package creation.
    exit $?
fi

# Remove uneeded QML stuff
if [ "$1" == "qmlui" ]; then
    rm -rf $BIN_DIR/QtQuick/Controls/FluentWinUI3
    #rm -rf $BIN_DIR/QtQuick/Controls/Fusion
    rm -rf $BIN_DIR/QtQuick/Controls/Imagine
    rm -rf $BIN_DIR/QtQuick/Controls/iOS
    rm -rf $BIN_DIR/QtQuick/Controls/Material
    rm -rf $BIN_DIR/QtQuick/Controls/Universal
    rm -rf $BIN_DIR/QtQuick/Particles
fi

cd ..

echo "Fix non-Qt dependencies..."
platforms/macos/fix_dylib_deps.sh $APP_DIR/Contents/Frameworks/libsndfile.1.dylib
if [ -f "$BIN_DIR/qlcplus" ]; then
    platforms/macos/fix_dylib_deps.sh $BIN_DIR/qlcplus
    platforms/macos/fix_dylib_deps.sh $BIN_DIR/qlcplus-fixtureeditor
else
    platforms/macos/fix_dylib_deps.sh $BIN_DIR/qlcplus-qml
fi

echo "Run macdeployqt..."
$QTDIR/bin/macdeployqt $APP_DIR

if [ -n "$SIGNATURE" ]; then
    # sign package with codesign (macdeployqt fails in that too)
    echo "Signing binaries..."
    ENTITLEMENTS="platforms/macos/qlcplus.entitlements"

    find $APP_DIR/Contents/Frameworks -type f | while read file; do
        codesign --force --sign "$SIGNATURE" --timestamp "$file"
    done

    find $APP_DIR/Contents/PlugIns -type f -name "*.dylib" | while read file; do
        codesign --force --sign "$SIGNATURE" --timestamp "$file"
    done

    find $BIN_DIR -type f | while read file; do
        codesign --force --sign "$SIGNATURE" --timestamp --entitlements $ENTITLEMENTS --options runtime "$file"
    done

    codesign --sign "$SIGNATURE" --timestamp --deep --entitlements $ENTITLEMENTS --options runtime $APP_DIR

    # workaround first time sign failure
    if [ -f "$BIN_DIR/qlcplus" ]; then
        codesign --force --sign "$SIGNATURE" --timestamp --entitlements $ENTITLEMENTS --options runtime $BIN_DIR/qlcplus
        codesign --force --sign "$SIGNATURE" --timestamp --entitlements $ENTITLEMENTS --options runtime $BIN_DIR/qlcplus-launcher
    else
        codesign --force --sign "$SIGNATURE" --timestamp --entitlements $ENTITLEMENTS --options runtime $BIN_DIR/qlcplus-qml
    fi

fi

# Create Apple Disk iMaGe from ~/QLC+.app/
OUTDIR=$PWD
cd platforms/macos/dmg
./create-dmg --volname "Q Light Controller Plus $VERSION" \
    --volicon $OUTDIR/resources/icons/qlcplus.icns \
    --background background.png \
    --window-size 400 300 \
    --window-pos 200 100 \
    --icon-size 64 \
    --icon "QLC+" 0 150 \
    --app-drop-link 200 150 \
    $OUTDIR/QLC+_$VERSION.dmg \
    $APP_DIR
cd -

if [ -n "$SIGNATURE" ]; then
    codesign --sign "$SIGNATURE" --timestamp $OUTDIR/QLC+_$VERSION.dmg
fi
