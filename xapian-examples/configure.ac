dnl Process this file with autoconf to produce a configure script.

dnl Need autoconf 2.50 or later for many features
dnl automake 1.6.3 requires autoconf 2.54
dnl autoconf 2.59 is what xapian-core requires, so be consistent
AC_PREREQ(2.59)
AC_INIT(xapian-examples, 0.8.3)dnl FIXME: bugreport addr as third argument
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR(delve.cc)

AC_CONFIG_HEADERS(config.h)

dnl Checks for programs.
AC_PROG_CXX

dnl Run tests using the C++ compiler.
AC_LANG_CPLUSPLUS

dnl Check for libxapian
AC_PROG_LIBTOOL
XO_LIB_XAPIAN(, AC_MSG_ERROR([Can't find Xapian library]))

dnl Some C++ compilers need a special switch to select ANSI C++ mode.  Do
dnl this early in configure so such settings are used for other tests, and
dnl put them in CXXFLAGS not AM_CXXFLAGS since AM_CXXFLAGS are only used in
dnl the Makefiles.
case "$CXX" in
  aCC|*/aCC)
    dnl Select ANSI mode (std:: namespace and ANSI scoping for i in:
    dnl   for (int i = 1; i < n; ++i) { ... }
    dnl (note the latest is controlled by: -ansi_for_scope,on if we need
    dnl drop -AA)
    dnl Note this means that user code must be built with -AA (which is
    dnl the default on Itanium, but not on PARISC:
    dnl  http://h21007.www2.hp.com/dspp/tech/tech_TechDocumentDetailPage_IDX/1,1701,5520,00.html
    dnl )
    CXXFLAGS="$CXXFLAGS -AA" ;;
  cxx|*/cxx)
    dnl Select ANSI mode (strict_ansi is needed for ANSI iostream, as -std ansi
    dnl gives a pre-standard AT&T-compatible version).
    dnl If this is a problem, try "-std ansi -D__USE_STD_IOSTREAM"
    CXXFLAGS="$CXXFLAGS -std strict_ansi" ;;
esac

dnl ******************************
dnl * Set special compiler flags *
dnl ******************************

dnl extra flags for building with gcc
if test yes = "$GXX"; then
  AM_CXXFLAGS="$AM_CXXFLAGS -Wall -W -Wno-long-long -fno-gnu-keywords"
fi

AC_SUBST(AM_CXXFLAGS)

dnl **************************
dnl * Build the output files *
dnl **************************

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
