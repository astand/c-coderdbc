#!/bin/bash
#  cd {root_of_repo}
#  ln -s ${PWD}/utl/format/pre-commit-astyle.sh .git/hooks/pre-commit
#  chmod u+x utl/format/pre-commit-astyle.sh

OPTIONS="--suffix=none \
--style=allman \
--attach-inlines \
--indent-switches \
--indent-preproc-define \
--indent=spaces=2 \
--min-conditional-indent=2 \
--break-blocks \
--pad-oper \
--pad-header \
--unpad-paren \
--align-pointer=type \
--align-reference=type \
--indent-modifiers \
--attach-classes \
--break-after-logical \
--keep-one-line-statements \
--indent-after-parens \
--add-braces \
--max-code-length=120 \
--max-continuation-indent=120"

RETURN=0
ASTYLE='astyle'
if [ $? -ne 0 ]; then
	echo "[!] astyle not installed. Unable to check source file format policy." >&2
	exit 1
fi

FILES=`find . | grep -P "^(?!(.*\/thirdparty|.*\/lib\/dbc|.*\/bin|.*\/doc|.*\/utl|.*\/tst|.*\/generated*)).*\.(c|cpp|h)$"`
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
