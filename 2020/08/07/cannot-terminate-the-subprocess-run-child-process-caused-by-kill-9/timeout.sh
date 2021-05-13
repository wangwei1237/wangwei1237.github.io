#!/bin/sh

for((i=0;;i++))
do
	echo "$i" > log 2>&1
	sleep 1
done

