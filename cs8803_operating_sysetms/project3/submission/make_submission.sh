#!/bin/bash

# Top level folder structure
SUBMISSION_FOLDER="kekatpure-rohan-pr3"
PACKAGE="$SUBMISSION_FOLDER.zip"

# Remove old version
echo $PACKAGE
rm -rfv "$PACKAGE"
rm -rfv "$SUBMISSION_FOLDER"

PART1="$SUBMISSION_FOLDER/part1"
PART2="$SUBMISSION_FOLDER/part2"

mkdir -p "$PART1"
mkdir -p "$PART2"
mkdir -p "$PART2/cached_files"

INDEX=(1 2)
for i in "${INDEX[@]}" ; do
	if [ $i -eq 1 ]; then
		DEST_DIR="$PART1"
	else
		DEST_DIR="$PART2"
	fi
	cp -v ../part$i/*.c "$DEST_DIR"
	cp -v ../part$i/*.h "$DEST_DIR"
	cp -v ../part$i/*.py "$DEST_DIR"
	cp -v ../part$i/*.sh "$DEST_DIR"
	cp -v ../part$i/workload.txt "$DEST_DIR"
	cp -v ../part$i/*.md "$DEST_DIR"
	cp -v ../part$i/Makefile "$DEST_DIR"	
	cp -v ../part$i/locals.txt "$DEST_DIR"
done

# make a zip file
zip -r "$SUBMISSION_FOLDER".zip "$SUBMISSION_FOLDER"

# remove submission folder
rm -rfv "$SUBMISSION_FOLDER"

