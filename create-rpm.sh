#!/bin/sh
#
# This script creates Q Light Controller RPM packages.

if [ -f Makefile ]; then
	echo Cleaning objects from previous compile
	make distclean
fi

if [ ! -d rpm ]; then
	echo ERROR: This script must be run from the top-level qlc source directory
	exit 1;
fi

VERSION=`head -1 debian/changelog | sed 's/.*(\(.*\)).*/\1/'`
SOURCES=qlc
RPMBUILD=~/rpmbuild

# Prepare RPM build directory hierarchy
if [ ! -f $RPMBUILD ]; then
	echo Creating RPMBUILD hierarchy in $RPMBUILD
	mkdir -p $RPMBUILD/SOURCES $RPMBUILD/SPECS $RPMBUILD/BUILD $RPMBUILD/SRPMS
	mkdir -p $RPMBUILD/RPMS/i386 $RPMBUILD/RPMS/athlon $RPMBUILD/RPMS/i486
	mkdir -p $RPMBUILD/RPMS/i586 $RPMBUILD/RPMS/i686 $RPMBUILD/RPMS/noarch
fi

# Copy the RPM spec file so that rpmbuild finds it
cp -f rpm/qlc.spec $RPMBUILD/SPECS

# Prepare a source tarball and move it under $RPMBUILD/SOURCES
echo "Packing sources into qlc-$VERSION.tar.gz..."
if [ -d /tmp/qlc-$VERSION ]; then
	rm -rf /tmp/qlc-$VERSION
fi

mkdir /tmp/qlc-$VERSION
rsync -aC . /tmp/qlc-$VERSION
tar --directory=/tmp -czf /tmp/qlc-$VERSION.tar.gz qlc-$VERSION
mv /tmp/qlc-$VERSION.tar.gz $RPMBUILD/SOURCES

cd $RPMBUILD/SPECS
QLC_VERSION=$VERSION rpmbuild -bb qlc.spec
if [ $? == 0 ]; then
	echo Packages created in $RPMBUILD/RPMS
fi
