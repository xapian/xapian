dnl Process this file with autoconf to produce a configure script.

dnl Need autoconf 2.50 or later for many features
dnl automake 1.6.3 requires autoconf 2.54
AC_PREREQ(2.54)
AC_INIT(xapian-queryserver, 0.8.0, richard@tartarus.org)
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR(source/queryserver.cpp)

AM_CONFIG_HEADER(config.h)

dnl Checks for programs.
AC_PROG_CC
AC_PROG_CXX

dnl Check for time functions.
AC_CHECK_FUNCS(gettimeofday)

dnl Check for libxapian
AC_PROG_LIBTOOL
XO_LIB_XAPIAN(, AC_MSG_ERROR([Can't find Xapian library]))

dnl ******************************
dnl * Set special compiler flags *
dnl ******************************

dnl extra flags for building with gcc
if test yes = "$GCC"; then
  AM_CFLAGS="$AM_CFLAGS -Wall -W"
  AM_CXXFLAGS="$AM_CXXFLAGS -Wall -W -fno-gnu-keywords"
fi

dnl **************************
dnl * Build the output files *
dnl **************************

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
