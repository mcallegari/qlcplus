#!/bin/bash
#
# This script creates Q Light Controller Plus debian packages and strips GIT
# folders from the source package.
#
# QLC+ 4 and QLC+ 5 have different upstream versions, and debian derives the
# version of every binary package from debian/changelog. They are therefore
# built one at a time, pointing debian/changelog at the changelog of the
# flavour being built. dpkg-buildpackage reads the changelog when it starts,
# so the symlink has to be in place before invoking it.
#
# Usage:
#   ./create-deb.sh          build both flavours
#   ./create-deb.sh v4       build QLC+ 4 only
#   ./create-deb.sh v5       build QLC+ 5 only

set -e

ROOT_DIR=$(cd "$(dirname "$0")" && pwd)
cd "$ROOT_DIR"

build_flavour() {
    local flavour=$1

    if [ ! -f "debian/changelog-$flavour" ]; then
        echo "Error: debian/changelog-$flavour not found" >&2
        exit 1
    fi

    echo "==> Building QLC+ ${flavour#v} packages"
    ln -sf "changelog-$flavour" debian/changelog

    # -us -uc: don't sign the source and changes files. Signing needs a GPG
    # key, and would abort the build when none is available. Sign afterwards
    # with debsign(1) if the packages have to be uploaded somewhere.
    dpkg-buildpackage -rfakeroot -I.git --jobs=auto -us -uc
}

case "${1:-all}" in
    v4) build_flavour v4 ;;
    v5) build_flavour v5 ;;
    all)
        build_flavour v4
        # Start the second build from a clean tree, so that the artifacts of
        # the first flavour don't end up in the second set of packages
        fakeroot debian/rules clean
        build_flavour v5
        ;;
    *)
        echo "Usage: $0 [v4|v5]" >&2
        exit 1
        ;;
esac

echo "==> Done"
