#!/bin/bash

ANT_ACTION=debug
if [ $# -ge 1 ];
then
    ANT_ACTION=$1
fi

WORK_DIR=fbbs/web
if [ $# -ge 2 ];
then
    WORK_DIR=$2
fi

# config
DEPLOY_DIR=/home/wwwroot/$WORK_DIR

git pull
ant $ANT_ACTION

# deploy new
cp -r webroot/* $DEPLOY_DIR
