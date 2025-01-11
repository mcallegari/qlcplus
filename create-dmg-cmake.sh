#!/bin/bash
#VERSION=$(head -1 debian/changelog | sed 's/.*(\(.*\)).*/\1/')
VERSION=$(grep -m 1 APPVERSION variables.pri | cut -d '=' -f 2 | sed -e 's/^[[:space:]]*//' | tr ' ' _ | tr -d '\r\n')

rm -rf build
mkdir build
cd build

# Build
if [ -n "$QTDIR" ]; then
    cmake -DCMAKE_PREFIX_PATH="$QTDIR/lib/cmake" ..
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
platforms/macos/fix_dylib_deps.sh ~/QLC+.app/Contents/Frameworks/libsndfile.1.dylib

echo "Run macdeployqt..."
$QTDIR/bin/macdeployqt ~/QLC+.app

#echo "Fix some more dependencies..."
#install_name_tool -change /usr/local/opt/fftw/lib/libfftw3.3.dylib @executable_path/../Frameworks/libfftw3.3.dylib ~/QLC+.app/Contents/MacOS/qlcplus
#install_name_tool -change /usr/local/opt/fftw/lib/libfftw3.3.dylib @executable_path/../Frameworks/libfftw3.3.dylib ~/QLC+.app/Contents/MacOS/qlcplus-fixtureeditor 

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
    ~/QLC+.app
cd -
