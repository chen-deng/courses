#!/bin/bash

PROJECT_FOLDER=./kekatpure-rohan-pr1

#remove the project zip
rm -f "$PROJECT_FOLDER".zip

# make the project folder
mkdir "$PROJECT_FOLDER"
mkdir "$PROJECT_FOLDER"/bin

# copy the project files
cp -v ./webclient.c "$PROJECT_FOLDER"
cp -v ./webserver.c "$PROJECT_FOLDER"
cp -v ./makefile "$PROJECT_FOLDER"
cp -v ./verifyhash.sh "$PROJECT_FOLDER"
cp -v ./README "$PROJECT_FOLDER"

# zip up the project
zip -r "$PROJECT_FOLDER".zip "$PROJECT_FOLDER"

# remove existing project folder again
rm -rfv "$PROJECT_FOLDER"
