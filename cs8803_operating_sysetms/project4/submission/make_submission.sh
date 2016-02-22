#!/bin/bash

# Top level folder structure
SOURCE_FOLDER="../single_threaded_rpc"
SUBMISSION_FOLDER="kekatpure-rohan-pr4"
PACKAGE="$SUBMISSION_FOLDER.zip"

# Remove old version
echo $PACKAGE
rm -rfv "$PACKAGE"
rm -rfv "$SUBMISSION_FOLDER"

mkdir -p "$SUBMISSION_FOLDER"


cp -v "$SOURCE_FOLDER"/*.c "$SUBMISSION_FOLDER"
cp -v "$SOURCE_FOLDER"/*.h "$SUBMISSION_FOLDER"
cp -v "$SOURCE_FOLDER"/*.x "$SUBMISSION_FOLDER"
cp -v "$SOURCE_FOLDER"/makefile "$SUBMISSION_FOLDER"

# make a zip file
zip -r "$SUBMISSION_FOLDER".zip "$SUBMISSION_FOLDER"

# remove submission folder
rm -rfv "$SUBMISSION_FOLDER"

