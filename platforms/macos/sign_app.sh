#!/bin/bash
# Sign an app bundle for distribution (hardened runtime, notarization ready).
# Code is signed from the innermost outwards, then the bundle itself (no --deep).
#
# Usage: sign_app.sh <path/to/App.app> <signing identity> [entitlements file]

APP_DIR=$1
IDENTITY=$2
ENTITLEMENTS=${3:-$(dirname "$0")/qlcplus.entitlements}

if [ ! -d "$APP_DIR" ] || [ -z "$IDENTITY" ]; then
    echo "Usage: $0 <path/to/App.app> <signing identity> [entitlements file]"
    exit 1
fi

NUM_CPUS=$(sysctl -n hw.ncpu 2>/dev/null || echo 4)
BIN_DIR="$APP_DIR/Contents/MacOS"
SIGN=(codesign --force --timestamp --options runtime --sign "$IDENTITY")

# loose dylibs (Frameworks, PlugIns, QML plugins)
echo "Signing dylibs..."
find "$APP_DIR/Contents" -type f -name "*.dylib" -print0 | xargs -0 -P $NUM_CPUS -n 8 "${SIGN[@]}" || exit 1

# Qt frameworks, signed as bundles
echo "Signing frameworks..."
find "$APP_DIR/Contents/Frameworks" -maxdepth 1 -name "*.framework" -print0 | xargs -0 -P $NUM_CPUS -n 4 "${SIGN[@]}" || exit 1

# secondary executables (everything in MacOS except the main one)
echo "Signing executables..."
MAIN_EXE=$(/usr/libexec/PlistBuddy -c "Print :CFBundleExecutable" "$APP_DIR/Contents/Info.plist")
find "$BIN_DIR" -type f ! -name "$MAIN_EXE" -print0 | xargs -0 -n 1 "${SIGN[@]}" --entitlements "$ENTITLEMENTS" || exit 1

# the bundle: signs the main executable and seals resources
echo "Signing app bundle..."
"${SIGN[@]}" --entitlements "$ENTITLEMENTS" "$APP_DIR" || exit 1

codesign --verify --deep --strict --verbose=1 "$APP_DIR" || exit 1
