#!/bin/bash
#VERSION=$(head -1 debian/changelog | sed 's/.*(\(.*\)).*/\1/')
VERSION=$(grep -m 1 APPVERSION variables.pri | cut -d '=' -f 2 | sed -e 's/^[[:space:]]*//' | tr ' ' _ | tr -d '\r\n')
APP_DIR=~/QLC+.app

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
    cmake -DCMAKE_PREFIX_PATH="$QTDIR/lib/cmake" -DCMAKE_OSX_DEPLOYMENT_TARGET=$CMAKE_OSX_DEPLOYMENT_TARGET ..
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

cd ..

echo "Fix non-Qt dependencies..."
platforms/macos/fix_dylib_deps.sh $APP_DIR/Contents/Frameworks/libsndfile.1.dylib
platforms/macos/fix_dylib_deps.sh $APP_DIR/Contents/MacOS/qlcplus
platforms/macos/fix_dylib_deps.sh $APP_DIR/Contents/MacOS/qlcplus-fixtureeditor

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

    find $APP_DIR/Contents/MacOS -type f | while read file; do
        codesign --force --sign "$SIGNATURE" --timestamp --entitlements $ENTITLEMENTS --options runtime "$file"
    done

    codesign --sign "$SIGNATURE" --timestamp --deep --entitlements $ENTITLEMENTS --options runtime $APP_DIR

    # workaround first time sign failure
    codesign --force --sign "$SIGNATURE" --timestamp --entitlements $ENTITLEMENTS --options runtime $APP_DIR/Contents/MacOS/qlcplus
    codesign --force --sign "$SIGNATURE" --timestamp --entitlements $ENTITLEMENTS --options runtime $APP_DIR/Contents/MacOS/qlcplus-launcher
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
