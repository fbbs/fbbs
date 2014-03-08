#!/bin/bash --norc

BBS_HOME=/home/bbs
BBS_CONFDIR=/etc/fbbs
BBS_DB=bbs
BBS_USR=bbs
BBS_GRP=bbs
BBS_UID=9999
BBS_GID=9999
PG_USR=postgres
PG_PORT=5432
REDIS_SOCKET=/var/run/redis/redis.sock
REDIS_HOST=127.0.0.1
REDIS_PORT=6379

curdir=$(dirname "$0")

if [[ $UID -ne 0 ]]; then
	echo "error: need root privilege to continue"
	exit 1
fi

if ! grep -q "^$PG_USR:" /etc/passwd; then
	echo "error: postgresql user [$PG_USR] not found"
	exit 1
fi

if ! grep -q "^$BBS_GRP:" /etc/group; then
	echo "creating group $BBS_GRP ..."
	groupadd -g "$BBS_GID" "$BBS_GRP"
fi

if ! grep -q "^$BBS_USR:" /etc/passwd; then
	echo "creating user $BBS_USR ..."
	useradd -u "$BBS_UID" -g "$BBS_GID" "$BBS_USR"
fi

if [[ -d "$BBS_HOME" ]]; then
	echo -n "warning: [$BBS_HOME] already exists. Overwrite? [N]"
	read ans
	ans=${ans:-N}
	case $ans in
		[Yy]) echo "removing old installation ... "; rm -rf "$BBS_HOME" ;;
		*) echo "aborted"; exit ;;
	esac
fi

echo "making dir $BBS_HOME ..."
mkdir -p "$BBS_HOME"

cp -a "$curdir/../bbshome/etc" "$BBS_HOME/etc"

echo "creating necessary directories ..."
dirs="mail home"
for x in $dirs; do
	mkdir "$BBS_HOME/$x"
	for i in {A..Z}; do
		mkdir "$BBS_HOME/$x/$i"
	done
done

dirs="boards vote"
boards="BBS_Help bmsecurity BM_Home boardsecurity club GoneWithTheWind newcomers Notepad Notice SysOp syssecurity usersecurity vote"
for x in $dirs; do
	mkdir "$BBS_HOME/$x"
	for i in $boards; do
		mkdir "$BBS_HOME/$x/$i"
	done
done

dirs="0Announce/bbslist bm board index logs post reclog so tmp temp"
for x in $dirs; do
	mkdir -p "$BBS_HOME/$x"
done
echo {001..999} | sed 's/ /\n/g' | xargs -I {} mkdir "$BBS_HOME/post/{}"

ln -s $BBS_HOME/lib/libadmintool.so $BBS_HOME/so/admintool.so

chown -R "$BBS_USR" "$BBS_HOME"
chgrp -R "$BBS_GRP" "$BBS_HOME"

if [[ -f /usr/bin/pg_ctlcluster ]]; then
	# Looks like debian
	PG_HOST='/var/run/postgresql'
fi

su -c "createuser -D -R -S $BBS_USR" $PG_USR
su -c "createdb -E UTF8 -O $BBS_USR $BBS_DB" $PG_USR

PG_SCHEMAS='schema board favboard friend payment session procedures'
for x in $PG_SCHEMAS; do
	su -c "psql -q -f $curdir/../pg/$x.sql" $BBS_USR
done

mkdir "$BBS_CONFDIR"
BBS_CONF="$BBS_CONFDIR/fbbs.conf"
echo "host = $PG_HOST" > "$BBS_CONF"
echo "port = $PG_PORT" >> "$BBS_CONF"
echo "dbname = $BBS_DB" >> "$BBS_CONF"
echo "user = $BBS_USR" >> "$BBS_CONF"
echo "password = 123456789" >> "$BBS_CONF"
echo "mdb = $REDIS_SOCKET" >> "$BBS_CONF"
echo "mdb_host = $REDIS_HOST" >> "$BBS_CONF"
echo "mdb_port = $REDIS_PORT" >> "$BBS_CONF"
