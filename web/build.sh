#!/bin/bash

BUILD_PATH=/home/output
if [ $# -ge 1 ];
then
    BUILD_PATH=$1
fi

WORK_DIR=fbbs/web
if [ $# -ge 2 ];
then
    WORK_DIR=$2
fi

mkdir $BUILD_PATH

ant release

DEPLOY_DIR=/home/wwwroot/$WORK_DIR
cp -r webroot/* $DEPLOY_DIR
find $DEPLOY_DIR -name .git -exec rm -rf {} \;

tar cvzf fbbs_src.tar.gz $DEPLOY_DIR/*
mv fbbs_src.tar.gz $BUILD_PATH
