#!/bin/sh
PROCESSFILE='server'

while [1]
do

if ps ax | grep -v grep | grep $PROCESSFILE > /dev/null
    then
    echo "$PROESSFILE is running, everything is fine"

else

echo "$PROCESSFILE is not running"
#start the process
#:wq

#./server &
fi
sleep 3000
done
