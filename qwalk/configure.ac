dnl Process this file with autoconf to produce a configure script.
AC_PREREQ(2.61)

dnl This is the only place where the package name and version appear
AC_INIT([qwalk], [git-taniwha])
AM_INIT_AUTOMAKE([foreign])

dnl LT_INIT messes with CFLAGS (evil bastard)
if test "x${CFLAGS-unset}" = xunset; then
	CFLAGS=""
fi
saved_CFLAGS="$CFLAGS"
dnl LT_INIT([win32-dll])
AM_PROG_LIBTOOL
CFLAGS="$saved_CFLAGS"

AC_REVISION([$Revision$]) dnl

AC_CONFIG_HEADERS([config.h])

AC_CANONICAL_BUILD
AC_CANONICAL_HOST

AC_LANG_C

if test "$x{AR-unset}" = xunset; then
	AR="ar"
fi
export AR
AC_SUBST(AR)
AC_SUBST(RANLIB)

AC_PROG_INSTALL
AC_PROG_CC
AC_PROG_CPP
AC_PROG_LN_S
AC_PROG_RANLIB
AM_PROG_AS

AC_SYS_LONG_FILE_NAMES
AC_EXEEXT
AC_OBJEXT


WINMM=""
case "$host_os" in
	cygwin*)
		WINMM="-lwinmm"
	;;
	mingw32*)
		WINMM="-lwinmm"
	;;
	*)
		dnl Checks for working -lm
		AC_CHECK_LIB(m, pow,, AC_MSG_ERROR([math library (-lm) appears broken]))
	;;
esac	

AC_CHECK_LIB(dl, dlopen)
AC_CHECK_LIB(GL, glFlush, [GL_LIBS=-lGL], [
	AC_MSG_CHECKING([for glFlush in -lopengl32])
	save_LIBS="$LIBS"
	LIBS=-lopengl32
	AC_LINK_IFELSE(
		[
			AC_LANG_PROGRAM([[
				#include <windows.h>
				#include <GL/gl.h>
			]], [[
				glFlush ();
			]])
		], [
			GL_LIBS="$LIBS"
			AC_MSG_RESULT([yes])
		], [
			AC_MSG_RESULT([no])
		]
	)
	LIBS="$save_LIBS"
])
AC_SUBST(GL_LIBS)

AM_PATH_SDL(1.2.0, HAVE_SDL=yes, HAVE_SDL=no)
AC_SUBST(HAVE_SDL)

dnl ==================================================================
dnl Checks for system type
dnl ==================================================================

dnl Checks for which system driver to use
AC_MSG_CHECKING(for system driver)
case "${host}" in
	i?86-*-mingw32*)
		SYSTYPE=WIN32
		AC_MSG_RESULT([Win32 driver])
		WIN32_LIBS=' $(NET_LIBS)'
		if test "x$host" != "x$build"; then
			case "$build_os" in
				cygwin*)
					WIN32_LIBS=" -L/usr/lib/w32api $WIN32_LIBS"
					;;
			esac
		fi

		;;
	i?86-*-cygwin*)
		SYSTYPE=WIN32
		AC_MSG_RESULT([Win32 driver])
		WIN32_LIBS=' $(NET_LIBS)'
		;;
	*)
		SYSTYPE=POSIX
		AC_MSG_RESULT([default POSIX driver])
		WIN32_LIBS=''
		;;
esac
AM_CONDITIONAL(SYSTYPE_WIN32, test "x$SYSTYPE" = "xWIN32")
AC_SUBST(WIN32_LIBS)

m4_include(config.d/build_control.m4)
m4_include(config.d/compiling.m4)

TOPSRC=`readlink -f ${srcdir}`
AC_SUBST(TOPSRC)

dnl Output files
AC_CONFIG_FILES(Makefile)
AC_OUTPUT

m4_include(config.d/result.m4)
