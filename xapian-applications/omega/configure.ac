dnl Process this file with autoconf to produce a configure script.

dnl Need autoconf 2.50 or later for many features
dnl automake 1.6.3 requires autoconf 2.54
dnl autoconf 2.59 is what xapian-core requires, so be consistent
AC_PREREQ(2.59)
AC_INIT(omega, 0.8.3)dnl FIXME: bugreport addr as third argument
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR(omega.cc)

AC_CONFIG_HEADERS(config.h)

dnl Checks for programs.
AC_PROG_CXX

dnl Run tests using the C++ compiler.
AC_LANG_CPLUSPLUS

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

dnl Check for time functions.
AC_CHECK_FUNCS(gettimeofday ftime)

dnl See if ftime returns void (as it does on mingw)
if test $ac_cv_func_ftime = yes ; then
  AC_TRY_COMPILE([#include <sys/timeb.h>],
    [struct timeb tp; int i = ftime(&tp);],
    ,
    AC_DEFINE(FTIME_RETURNS_VOID, 1, [Define if ftime returns void]))
fi

AC_CHECK_FUNCS(posix_fadvise)

dnl mingw has _snprintf
AC_CHECK_FUNCS(snprintf _snprintf)
if test $ac_cv_func_snprintf$ac_cv_func__snprintf = noyes ; then
  AC_DEFINE(snprintf, _snprintf,
	    [Define snprintf to whatever it's called on this platform])
  AC_DEFINE(HAVE_SNPRINTF, 1)
fi

dnl Check for libxapian
AC_PROG_LIBTOOL
XO_LIB_XAPIAN(, AC_MSG_ERROR([Can't find Xapian library]))

dnl Disabled XML stuff as we don't currently build omindex-config
dnl dnl Check for libxml or libxml2.  We do this by looking for xml-config.
dnl AC_PATH_PROG(XML_CONFIG_PATH, xml2-config)
dnl if test -n "$XML_CONFIG_PATH" ; then
dnl   HAVE_LIBXML2=yes
dnl   AC_DEFINE(HAVE_LIBXML2,, [Define if libxml2 is available.])
dnl else
dnl   HAVE_LIBXML2=no
dnl   AC_PATH_PROG(XML_CONFIG_PATH, xml-config)
dnl fi
dnl if test -n "$XML_CONFIG_PATH" ; then
dnl   AC_DEFINE(HAVE_LIBXML,, [Define if libxml or libxml2 is available.])
dnl fi
dnl AM_CONDITIONAL(HAVE_LIBXML2, test yes = "$HAVE_LIBXML2")
dnl 
dnl if test -n "$XML_CONFIG_PATH" ; then
dnl     AC_MSG_CHECKING([libxml flags])
dnl     LIBXML_CFLAGS="`$XML_CONFIG_PATH --cflags`"
dnl     LIBXML_LIBS="`$XML_CONFIG_PATH --libs`"
dnl 
dnl     AC_LANG_SAVE
dnl     AC_LANG_C
dnl     SAVE_CFLAGS="$CFLAGS"
dnl     CFLAGS="$CFLAGS $LIBXML_CFLAGS"
dnl     AC_TRY_COMPILE([
dnl #include <parser.h>
dnl #include <valid.h>
dnl ], [ xmlValidCtxt ctxt;
dnl      xmlDocPtr doc;
dnl      xmlValidateDocument(&ctxt, doc);
dnl      ], [have_libxml_valid=yes], [have_libxml_valid=no])
dnl     CFLAGS="$SAVE_CFLAGS"
dnl     AC_LANG_RESTORE
dnl     AC_MSG_RESULT("$LIBXML_CFLAGS")
dnl fi
dnl AC_SUBST(LIBXML_CFLAGS)
dnl AC_SUBST(LIBXML_LIBS)
dnl if test yes = "$have_libxml_valid"; then
dnl AC_DEFINE(HAVE_LIBXML_VALID,, [Define if libxml has validation available ])
dnl else
dnl AC_MSG_WARN(libxml doesn't have xmlValidateDocument: disabling validation.)
dnl fi

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

AC_CONFIG_FILES([Makefile omega.spec])
AC_OUTPUT
