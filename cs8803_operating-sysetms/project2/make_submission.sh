#!/bin/bash
SUBMISSION_FOLDER="kekatpure-rohan-pr2"
rm -rfv "$SUBMISSION_FOLDER"
mkdir "$SUBMISSION_FOLDER"

# copy files
cp ./report/report.pdf "$SUBMISSION_FOLDER"
cp ./configuration.txt "$SUBMISSION_FOLDER"
cp ./analyser.py "$SUBMISSION_FOLDER"

# make a zip file
zip -r "$SUBMISSION_FOLDER".zip "$SUBMISSION_FOLDER"

# remove submission folder
rm -rfv "$SUBMISSION_FOLDER"
