#!/bin/bash
#export PATH=.:$PATH
#export OUT_HOST_ROOT=.

echo "----------unzip the update------------"

update_file=$1

if [ ! -f "$update_file" ]; then
	echo "$update_file is not exist"
	exit 1
fi
echo "updaet_file = " "$update_file"

unzip -o -q $update_file -d /tmp/update_tmp/

if [ ! -d "/tmp/update_tmp"]; then
	echo "/tmp/update_tmp is not exist"
	exit 1
fi

ls /tmp/update_tmp/
rm $update_file

cd /tmp/update_tmp

echo "pwd:"
pwd

echo "----------zip the update------------"
zip -r -q update.zip ./*
if [ ! -d "update.zip"]; then
	echo "/tmp/update.zip is not exist"
	exit 1
fi
mv update.zip $update_file

if [ ! -d "$update_file"]; then
	echo "$update_file is not exist"
	exit 1
else
	echo "restore ok"
	rm -r /tmp/update_tmp
fi
