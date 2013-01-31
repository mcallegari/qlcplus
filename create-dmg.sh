#!/bin/bash
VERSION=`head -1 debian/changelog | sed 's/.*(\(.*\)).*/\1/'`

#if [ -e "/var/db/receipts/com.apple.pkg.Rosetta.plist" ]; then
#    echo Rosetta installed. OK to continue.
#else
#    echo You need to install Rosetta from your Snow Leopard Install DVD!
#    exit 1
#fi

# Compile translations
./translate.sh

# Build
qmake -spec macx-g++
make distclean
qmake -spec macx-g++
make
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
cd dmg
./create-dmg --volname "Q Light Controller Plus $VERSION" \
	     --background background.png \
	     --window-size 300 225 \
	     --icon-size 128 --icon "qlcplus" 150 16 \
	     QLC+-$VERSION.dmg \
	     ~/QLC+.app
cd ..
