dnl Process this file with autoconf to produce a configure script.

dnl Need autoconf 2.50 or later for many features
dnl automake 1.6.3 requires autoconf 2.54
AC_PREREQ(2.54)
AC_INIT(xapian-examples, 0.8.2)dnl FIXME: bugreport addr as third argument
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR(delve.cc)

AM_CONFIG_HEADER(config.h)

dnl Checks for programs.
AC_PROG_CC
AC_PROG_CXX

dnl Check for libxapian
AC_PROG_LIBTOOL
XO_LIB_XAPIAN(, AC_MSG_ERROR([Can't find Xapian library]))

dnl ******************************
dnl * Set special compiler flags *
dnl ******************************

dnl extra flags for building with gcc
if test yes = "$GCC"; then
  AM_CFLAGS="$AM_CFLAGS -Wall -W -Wno-long-long"
  AM_CXXFLAGS="$AM_CXXFLAGS -Wall -W -Wno-long-long -fno-gnu-keywords"
fi

dnl modify flags for building with stlport
if test yes = "$use_stlport"; then
  if test yes = "$GCC"; then
    AM_CXXFLAGS="-nostdinc++ $AM_CXXFLAGS"
  fi
  AM_CXXFLAGS="$STLPORT_INCLUDE $AM_CXXFLAGS"
  LIBS="$STLPORT_LIBS $LIBS"
fi

dnl **************************
dnl * Build the output files *
dnl **************************

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
