dnl Process this file with autoconf to produce a configure script.

dnl Need autoconf 2.50 or later for many features
AC_PREREQ(2.50)
AC_INIT(xapview, 0.1-cvs)dnl FIXME: bugreport addr as third argument
AM_INIT_AUTOMAKE(xapview, 0.1-cvs)dnl remove args once we require am1.6
AC_CONFIG_SRCDIR(xapview.cpp)

AM_CONFIG_HEADER(config.h)

dnl Checks for programs.
AC_PROG_CC
AC_PROG_CXX

dnl Check for libxapian
AC_PROG_LIBTOOL
XO_PATH_XAPIAN(, AC_MSG_ERROR([Can't find Xapian library]))

dnl Checks for gtk / glib / glade
AM_PATH_GTK(1.2.0, , AC_MSG_ERROR([Can't find GTK+ 1.2.0 or newer]))
AM_PATH_GLIB(1.2.0, , AC_MSG_ERROR([Can't find GLIB 1.2.0 or newer]))
AM_PATH_LIBGLADE(, AC_MSG_ERROR([Can't find libglade]))

dnl ******************************
dnl * Set special compiler flags *
dnl ******************************

dnl extra flags for building with gcc
if test yes = "$GCC"; then
  AM_CFLAGS="$AM_CFLAGS -Wall -Wunused -Wno-long-long"
  AM_CXXFLAGS="$AM_CXXFLAGS -Wall -Wunused -Wno-long-long -fno-gnu-keywords"
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

AC_OUTPUT(Makefile)
