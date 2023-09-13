#!/bin/bash

# Define the source and destination directories
SOURCE_DIR="."
DEST_DIR="./build"
if [ -d "$2" ]; then
  DEST_DIR="$2"
fi

echo "Using the destination directory $DEST_DIR"

if [ ! -d $DEST_DIR/resources/ ]; then
  mkdir -p $DEST_DIR/resources/
fi

# Copy resources directories necessary for unittest
cp -r $SOURCE_DIR/resources/colorfilters $DEST_DIR/resources
cp -r $SOURCE_DIR/resources/fixtures $DEST_DIR/resources
cp -r $SOURCE_DIR/resources/gobos $DEST_DIR/resources
cp -r $SOURCE_DIR/resources/icons $DEST_DIR/resources
cp -r $SOURCE_DIR/resources/inputprofiles $DEST_DIR/resources
cp -r $SOURCE_DIR/resources/rgbscripts $DEST_DIR/resources
cp -r $SOURCE_DIR/resources/schemas $DEST_DIR/resources

# Find all files necessary for tests recursively in the source directory and copy to destination directory
for file in $(find $SOURCE_DIR -path $DEST_DIR -prune -o \( -name "test.sh" -o -name "*.xml*" \)); do

    # Get the directory of the file (excluding the "./" prefix)
    dir=$(dirname ${file#./})

    # Create the destination directory if it doesn't exist
    mkdir -p $DEST_DIR/$dir

    # Move the file to the new destination
    cp $file $DEST_DIR/$dir/
done

cp $SOURCE_DIR/unittest.sh $DEST_DIR/

pushd $DEST_DIR
./unittest.sh $1
popd