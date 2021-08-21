AC_MSG_RESULT([
	qwalk has been configured successfully.

	Build type         :$BUILD_TYPE
	Tools support      :${TOOLS_TARGETS- no}
	Compiler version   : $CCVER
	Compiler flags     : $CFLAGS
])

if test -d $srcdir/.git; then
	echo "WARNING: Hackers at work, watch for falling bits of code."
	echo "(This is from a development tree. Expect problems)"
	echo
fi
