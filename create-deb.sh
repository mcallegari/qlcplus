#!/bin/bash
#
# This script creates Q Light Controller debian packages and strips subversion
# folders from the source package.

# Compile translations
./translate.sh

# Build package
dpkg-buildpackage -rfakeroot -I.svn
