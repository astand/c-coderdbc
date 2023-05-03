#!/bin/bash

OPTIONS="--options=conf/astylerc"

RETURN=0
ASTYLE='./astyle'
if [ $? -ne 0 ]; then
	echo "[!] astyle not installed. Unable to check source file format policy." >&2
	exit 1
fi

FILES=`find . | grep -P "^(.*\/src).*\.(c|cpp|h)$"`
for FILE in $FILES; do
  # compare files
	# $ASTYLE $OPTIONS < $FILE | cmp -s $FILE -
	# if [ $? -ne 0 ]; then
	# 	echo "[!] $FILE does not respect the agreed coding style." >&2
	# 	RETURN=1
	# fi
  # forcibly format file
  $ASTYLE $OPTIONS $FILE
  # git add $FILE
  echo $FILE
done

if [ $RETURN -eq 1 ]; then
	echo "" >&2
	echo "Make sure you have run astyle with the following options:" >&2
	echo $OPTIONS >&2
fi

exit $RETURN
