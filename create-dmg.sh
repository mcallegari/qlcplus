#!/bin/bash
#VERSION=$(head -1 debian/changelog | sed 's/.*(\(.*\)).*/\1/')
VERSION=$(grep -m 1 APPVERSION variables.pri | cut -d '=' -f 2 | sed -e 's/^[[:space:]]*//' | tr ' ' _ | tr -d '\r\n')

# Compile translations
./translate.sh

# Build
if [ -n "$QTDIR" ]; then
    $QTDIR/bin/qmake $1
    make distclean
    $QTDIR/bin/qmake $1
else
    qmake -spec macx-g++
    make distclean
    qmake -spec macx-g++
fi

make -j4

if [ ! $? -eq 0 ]; then
    echo Compiler error. Aborting package creation.
    exit $?
fi

# Install to ~/QLC+.app/
make install
if [ ! $? -eq 0 ]; then
    echo Installation error. Aborting package creation.
    exit $?
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
	     ~/QLC+.app
cd -
