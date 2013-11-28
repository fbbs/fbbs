BBS_HOME=/home/bbs
BBS_USR=bbs
BBS_GRP=bbs
BBS_UID=9999
BBS_GID=9999

curdir=$(dirname "$0")

if [[ $UID -ne 0 ]]; then
	echo "error: need root privilege to continue"
	exit
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

dirs="0Announce/bbslist bm brdidx index logs post reclog so tmp temp"
for x in $dirs; do
	mkdir -p "$BBS_HOME/$x"
done

chown -R "$BBS_USR" "$BBS_HOME"
chgrp -R "$BBS_GRP" "$BBS_HOME"
