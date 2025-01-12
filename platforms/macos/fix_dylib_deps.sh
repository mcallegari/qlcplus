#!/bin/bash

if [ -z "$1" ]; then
    echo "Usage: $0 <source_dylib>"
    exit 1
fi

HOMEBREW_PREFIX=`brew --prefix`
SRC_DYLIB="$1"
FRAMEWORKS_DIR=~/QLC+.app/Contents/Frameworks

echo "Processing $SRC_DYLIB... (Homebrew at $HOMEBREW_PREFIX)"

# Ensure Frameworks directory exists
mkdir -p "$FRAMEWORKS_DIR"

# Extract the source dylib's full path from the second line of otool output
dylib_full_path=$(otool -L "$SRC_DYLIB" | sed -n '2p' | awk '{print $1}')
dylib_dir=$(dirname "$dylib_full_path")

echo "Source dylib full path: $dylib_full_path"
echo "Source dylib directory: $dylib_dir"

# Process each dependency reported by otool
otool -L "$SRC_DYLIB" | grep opt | awk '{print $1}' | while read -r dep; do
    subst=$(basename "$dep")

    if [[ "$dep" == *"$HOMEBREW_PREFIX"* ]]; then
        echo "Found Homebrew library: $dep ($subst)"

        # Check if the dependency is already in the target Frameworks directory
        if [ ! -f "$FRAMEWORKS_DIR/$subst" ]; then
            echo "Dependency missing: $subst. Adding it to target..."
            cp "$dep" "$FRAMEWORKS_DIR/"
        fi

        # Update the source dylib's reference to the dependency
        install_name_tool -change "$dep" "@executable_path/../Frameworks/$subst" "$SRC_DYLIB"
    
    elif [[ "$dep" == @loader_path* ]]; then
        echo "Found @loader_path reference: $dep ($subst)"
        
        # Resolve the actual path based on the source dylib's directory
        resolved_path=$(realpath "$dylib_dir/${dep#@loader_path/}")

        echo "Resolved path for @loader_path: $resolved_path"

        if [ -f "$resolved_path" ]; then
            # Check if the dependency is already in the target Frameworks directory
            if [ ! -f "$FRAMEWORKS_DIR/$subst" ]; then
                echo "Dependency missing: $subst. Copying resolved @loader_path dependency..."
                cp "$resolved_path" "$FRAMEWORKS_DIR/"
            fi

            # Update the source dylib's reference to the dependency
            install_name_tool -change "$dep" "@executable_path/../Frameworks/$subst" "$SRC_DYLIB"
        else
            echo "Warning: Resolved path does not exist: $resolved_path"
        fi
    fi
done
