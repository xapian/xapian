dnl Process this file with autoconf to produce a configure script.

AC_INIT([xapian-extras], [1.1.0], [http://xapian.org/bugs])
AC_PREREQ([2.63])
AM_INIT_AUTOMAKE([1.10.1 -Wportability tar-ustar])

AC_CONFIG_SRCDIR([imgseek/src/haar.cc])

AC_CONFIG_HEADERS([config.h])

AC_CONFIG_MACRO_DIR([m4])

LT_PREREQ([2.2.6])
LT_INIT

dnl Check for xapian-core.
XO_LIB_XAPIAN

dnl disable "maintainer only" rules by default
AM_MAINTAINER_MODE

dnl Checks for programs.
AC_PROG_CXX
AC_PROG_CC

# Checks for libraries.
AC_CHECK_LIB([jpeg], [jpeg_std_error])
PKG_CHECK_MODULES([QT], [qt-mt])

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.
AC_CHECK_FUNCS([sqrt])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
