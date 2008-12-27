#! /bin/sh
echo "========================="
echo "clear web process ..."
echo "========================="
for pid in `ps -A | grep apache2 | awk '{ print $1 }'`
do
   kill -9 $pid
   echo "kill apache2 (pid:$pid)"
done
for pid in `ps -A | grep bbslogin | awk '{ print $1 }'`
do
   kill -9 $pid
   echo "kill bbslogin (pid:$pid)"
done

		 
	  


echo "========================="
echo "clear bbsd process ..."
echo "========================="
for pid in `ps -A | grep bbsd | awk '{ print $1 }'`
do
   kill -9 $pid
   echo "kill bbsd (pid:$pid)"
done

echo "========================="
echo "clear chatd process ..."
echo "========================="
for pid in `ps -A | grep chatd | awk '{ print $1 }'`
do
   kill -9 $pid
   echo "kill chatd (pid:$pid)"
done

echo "========================="
echo "clear bbsnet process ..."
echo "========================="
for pid in `ps -A | grep bbsnet | awk ' { print $1 }'`
do
	kill -9 $pid
	echo "kill bbsnet (pid:$pid)"
done

			

echo "========================="
echo "clear miscd process ..."
echo "========================="
/home/bbs/bin/miscd flushed
rm /home/bbs/.PASSWDS.bak
cp /home/bbs/.PASSWDS /home/bbs/.PASSWDS.bak
for pid in `ps -A | grep miscd | awk '{ print $1 }'`
do
   kill -9 $pid
   #   echo "kill bbsd (pid:$pid)"
   done

echo "============================"
echo "remove bbs share memory ..."
echo "============================"
for shm_id in `ipcs -m| grep bbs | awk '{ print $2 }'`
do
    ipcrm -m $shm_id
done

/home/bbs/bin/miscd daemon
/home/bbs/bin/bbsd 12345
/home/bbs/bin/chatd
