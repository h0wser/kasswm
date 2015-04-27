#/bin/bash

# Sup
# This script runs Xephyr and restarts build/kasswm on changes (compiles) 
# To simplify testing

BIN_PATH="build/kasswm"
RES="800x600"

Xephyr -screen $RES -br :1 &
sleep 1

while true;
do
	ATIME=$(stat -c %Z $BIN_PATH)

	if [[ "$ATIME" != "$LTIME" ]]
	then
		if [[ -n "$PID" ]]
		then
			kill $PID
		fi

		echo "===== Restarting $BIN_PATH ====="
		DISPLAY=:1 $BIN_PATH  "$1" &
		DISPLAY=:1 xsetroot -solid gray
		DISPLAY=:1 xrdb -m ~/.Xresources
		PID="$!"
		LTIME=$ATIME
	fi
	sleep 3
done
