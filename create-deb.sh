#!/bin/bash
#
# This script creates Q Light Controller Plus debian packages and strips GIT
# folders from the source package.

# Build package
dpkg-buildpackage -rfakeroot -I.git --jobs=auto
