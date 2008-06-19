#! /bin/sh

echo "============================"
echo "remove bbs share memory ..."
echo "============================"
for shm_id in `ipcs | grep bbs | awk '{ print $2 }'`
do
    ipcrm -m $shm_id
done
echo "========================="
echo "clear bbsd process ...   "
echo "========================="
for pid in `ps -A | grep bbsd | awk '{ print $1 }'`
do
   kill -9 $pid
   echo "kill bbsd (pid:$pid)"
done

for pid in `ps -A | grep chatd | awk '{ print $1 }'`
do
	kill -9 $pid
	echo "kill chatd (pid:$pid)"
done
