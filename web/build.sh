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
rm -rf $DEPLOY_DIR/*
cp -r webroot/* $DEPLOY_DIR
find $DEPLOY_DIR -name .git -exec rm -rf {} \;

tar -zcvf fbbs_src.tar.gz webroot/* --exclude=webroot/bbs
mv fbbs_src.tar.gz $BUILD_PATH
echo "Released package download:"
echo "wget -nH http://work:work@output.littlepan.com/fbbs_src.tar.gz"
