dnl Process this file with autoconf to produce a configure script.

AC_INIT([xapian-profiler], [1.5.0], [https://xapian.org/bugs])
AC_CONFIG_MACRO_DIRS([m4])
AM_INIT_AUTOMAKE([foreign -Wall -Wportability -Werror])
dnl We don't want to build static libraries, but without this we get a
dnl warning from autoreconf.
AM_PROG_AR
LT_INIT([disable-static])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_SRCDIR([profiler.c])

AC_PROG_CC

AC_CHECK_SIZEOF([int])
AC_CHECK_SIZEOF([long])
AC_CHECK_SIZEOF([long long])
AC_CHECK_SIZEOF([off_t], [], [#include <sys/types.h>])

dnl Check for 64-bit versions (AC_CHECK_FUNCS only checks linkage which matches
dnl what we need as we want to be able to find the symbol with dlopen).
AC_CHECK_FUNCS([open64 pread64 pwrite64])

AC_CHECK_FUNCS([fsync fdatasync])

AC_SEARCH_LIBS([dlopen], [dl])

dnl Enable extra warning flags for building with GCC.
if test yes = "$GCC"; then
  AM_CXXFLAGS="$AM_CXXFLAGS -Wall -W -Wshadow -Wpointer-arith -Wcast-align"
fi
AC_SUBST([AM_CXXFLAGS])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
