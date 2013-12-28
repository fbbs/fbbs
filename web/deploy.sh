#!/bin/bash

DEPLOY_PATH=/home/wwwroot/fbbs/web
TMP_DIR=tmp

rm -rf $DEPLOY_PATH/*
if [ -d $TMP_DIR ]
then
	rm -rf $TMP_DIR
fi

JS_ROOT=js
CSS_ROOT=css
TPL_ROOT=tpl
JS_COMP=/home/build/closure-compiler/compiler.jar
CSS_COMP=/home/build/yuicompressor/yuicompressor.jar
TPL_COMP=
SMASHER_FILE=./smasher/smasher.xml
TMP_DIRS="$TMP_DIR $TMP_DIR/$JS_ROOT $TMP_DIR/$CSS_ROOT $TMP_DIR/$TPL_ROOT"
CLEAN_FILE="$DEPLOY_PATH/$TMP_DIR $DEPLOY_PATH/$JS_ROOT $DEPLOY_PATH/$CSS_ROOT $DEPLOY_PATH/$TPL_ROOT $DEPLOY_PATH/smasher $DEPLOY_PATH/build $DEPLOY_PATH/develop.php $DEPLOY_PATH/.htaccess $DEPLOY_PATH/deploy.sh"
JS_FILES=
CSS_FILES=
TPL_FILES=

while getopts "d:j:c:t:" arg
do
	case $arg in
		d)
			DEPLOY_PATH=$OPTARG
			;;
		j)
			JS_COMP=$OPTARG
			;;
		c)
			CSS_COMP=$OPTARG
			;;
		t)
			TPL_COMP=$OPTARG
			;;
	esac
done

mkdir $TMP_DIRS

cat $SMASHER_FILE | while read LINE
do
	if echo $LINE | grep "<group " > /dev/null
	then
		GROUP=${LINE#* id=\"}
		GROUP=${GROUP%%\"*}
		JS_FILES=
		CSS_FILES=
		TPL_FILES=
		echo "INFO: Group $GROUP start"
	fi
	NAME=
	TYPE=
	if echo $LINE | grep "<file " > /dev/null
	then
		TYPE=${LINE#* type=\"}
		TYPE=${TYPE%%\"*}
		NAME=${LINE#* src=\"}
		NAME=${NAME%%\"*}
		echo "INFO: File type $TYPE name $NAME"
		case "$TYPE" in
			"js" )
				JS_FILES=${JS_FILES}" "${NAME}
			;;
			"css" )
				CSS_FILES=${CSS_FILES}" "${NAME}
			;;
			"tpl" )
				TPL_FILES=${TPL_FILES}" "${NAME}
			;;
		esac
	fi
	if echo $LINE | grep "<dir " > /dev/null
	then
		TYPE=${LINE#* type=\"}
		TYPE=${TYPE%%\"*}
		NAME=${LINE#* src=\"}
		NAME=${NAME%%\"*}
		echo "INFO: Dir type $TYPE name $NAME start"
		case "$TYPE" in
			"js" )
				if [ -d $NAME ]
				then
					for FILE in `ls $NAME`
					do
						if [ -f ${NAME}${FILE} ]
						then
							JS_FILES=${JS_FILES}" "${NAME}${FILE}
							echo "INFO: File $FILE"
						fi
					done
				fi
			;;
			"css" )
				if [ -d $NAME ]
				then
					for FILE in `ls $NAME`
					do
						if [ -f ${NAME}${FILE} ]
						then
							CSS_FILES=${CSS_FILES}" "${NAME}${FILE}
							echo "INFO: File $FILE"
						fi
					done
				fi
			;;
			"tpl" )
				if [ -d $NAME ]
				then
					for FILE in `ls $NAME`
					do
						if [ -f ${NAME}${FILE} ]
						then
							TPL_FILES=${TPL_FILES}" "${NAME}${FILE}
							echo "INFO: File $FILE"
						fi
					done
				fi
			;;
		esac
		echo "INFO: Dir type $TYPE name $NAME end"
	fi
	if echo $LINE | grep "</group>" > /dev/null
	then
		echo "INFO: Group $GROUP end"
		echo "INFO: Compile group $GROUP start"
		if [ "$JS_FILES" ]
		then
			echo "INFO: Compiler params js $JS_FILES"
			java -jar $JS_COMP --js $JS_FILES --js_output_file $TMP_DIR/$JS_ROOT/$GROUP.js --charset=utf-8
		fi
		if [ "$CSS_FILES" ]
		then
			for CSS in $CSS_FILES
			do
				echo "INFO: Compiler params css $CSS"
				java -jar $CSS_COMP --type css --charset utf-8 -v $CSS >> $TMP_DIR/$CSS_ROOT/$GROUP.css
			done
		fi
		if [ "$TPL_FILES" ]
		then
			echo "INFO: Compiler params tpl $TPL_FILES"
			cat $TPL_FILES | sed 's/^\s\s*//g' | sed 's/\s\s*$//g' > $TMP_DIR/$TPL_ROOT/$GROUP.html
		fi
		echo "INFO: Compile group $GROUP end"
	fi
done

for JS in `ls $JS_ROOT`
do
	if [ -f $JS_ROOT/$JS ]
	then
		java -jar $JS_COMP --js $JS_ROOT/$JS --js_output_file $TMP_DIR/$JS_ROOT/$JS --charset=utf-8
		echo "INFO: Compile js $JS"
	fi
done

for CSS in `ls $CSS_ROOT`
do
	if [ -f $CSS_ROOT/$CSS ]
	then
		java -jar $CSS_COMP --type css --charset utf-8 -v $CSS_ROOT/$CSS > $TMP_DIR/$CSS_ROOT/$CSS
		echo "INFO: Compile css $CSS"
	fi
done

cp -r * $DEPLOY_PATH
rm -rf $CLEAN_FILE
cp -r $TMP_DIR/* $DEPLOY_PATH
cp -r css/decorator $DEPLOY_PATH/css
cp -r css/font $DEPLOY_PATH/css
find $DEPLOY_PATH -type d -name ".svn"|xargs rm -rf
find $DEPLOY_PATH -type d -name ".git"|xargs rm -rf
rm -rf $TMP_DIR

echo "BUILD SUCCESSFULLY!"

exit 0
