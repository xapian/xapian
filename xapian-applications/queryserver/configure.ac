dnl Process this file with autoconf to produce a configure script.

dnl Need autoconf 2.50 or later for many features
dnl automake 1.6.3 requires autoconf 2.54
AC_PREREQ(2.54)
AC_INIT(xapian-queryserver, 0.8.4, richard@tartarus.org)
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR(source/queryserver.cpp)

AC_CONFIG_HEADERS(config.h)

dnl Check for libxapian
AC_PROG_LIBTOOL
XO_LIB_XAPIAN(, AC_MSG_ERROR([Can't find Xapian library]))
dnl We want XAPIAN_CXXFLAGS to be used for configure tests
CXXFLAGS="$CXXFLAGS $XAPIAN_CXXFLAGS"

dnl Checks for programs.
AC_PROG_CXX

dnl Run tests using the C++ compiler.
AC_LANG_CPLUSPLUS

dnl mingw (for instance) lacks ssize_t
AC_CHECK_TYPE(ssize_t, long)

dnl check for socklen_t
TYPE_SOCKLEN_T

dnl Check for halibut. (Needed to make some of the documentation)
AC_PATH_PROG([HALIBUT], [halibut], [$MISSING halibut])

dnl ******************************
dnl * Set special compiler flags *
dnl ******************************

dnl extra flags for building with gcc
if test yes = "$GXX"; then
  AM_CXXFLAGS="$AM_CXXFLAGS -Wall -W -fno-gnu-keywords"
fi

dnl **************************
dnl * Build the output files *
dnl **************************

AC_CONFIG_FILES([Makefile docs/Makefile])
AC_OUTPUT
