dnl Process this file with autoconf to produce a configure script.

dnl Need autoconf 2.50 or later for many features
dnl automake 1.6.3 requires autoconf 2.54
dnl autoconf 2.59 is what xapian-core requires, so be consistent
AC_PREREQ(2.59)
AC_INIT(omega, 0.9.4)dnl FIXME: bugreport addr as third argument
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR(omega.cc)

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

dnl Check for time functions.
AC_CHECK_FUNCS(gettimeofday ftime)

dnl See if ftime returns void (as it does on mingw)
if test $ac_cv_func_ftime = yes ; then
  AC_TRY_COMPILE([#include <sys/timeb.h>],
    [struct timeb tp; int i = ftime(&tp);],
    ,
    AC_DEFINE(FTIME_RETURNS_VOID, 1, [Define if ftime returns void]))
fi

dnl Check for lstat (not available under mingw for example).
AC_CHECK_FUNCS(lstat)

AC_CHECK_FUNCS(posix_fadvise mmap)

AC_MSG_CHECKING(for working ISO C90 conforming snprintf)
ac_cv_func_snprintf_noniso="no"
dnl mingw has _snprintf
for func in snprintf _snprintf ; do
  dnl Check that snprintf actually works as it's meant to.
  dnl
  dnl Linux 'man snprintf' warns:
  dnl  Linux libc4.[45] does not have a snprintf, but provides a libbsd that
  dnl  contains an snprintf equivalent to sprintf, i.e., one that ignores the
  dnl  size argument.  Thus, the use of snprintf with early libc4 leads to
  dnl  serious security problems.
  dnl
  dnl It also warns that glibc < 2.0.6 (and presumably other pre-C90
  dnl implementations) return -1 when truncating so check that we get the
  dnl ISO C90 semantics for the returned length when truncating.  If we
  dnl have a working snprintf but with non-ISO return semantics, handle
  dnl that case separately as it may still be useful in many cases.
  AC_RUN_IFELSE([
    AC_LANG_PROGRAM(
      [[
#include <stdio.h>
#include <string.h>
      ]],
      dnl Return different exit status for each error so we can see which
      dnl check failed by consulting config.log.
      [[
	char buffer[4] = "abc";
	int exit_status = 0;
	int res1 = $func(buffer, 2, "%s", "XYZ");
	if (memcmp(buffer, "X\0c", 4) != 0) return 2;
	int res2 = $func(buffer, 2, "%x", 0x12);
	if (memcmp(buffer, "1\0c", 4) != 0) return 3;
	if (res1 == -1 && res2 == -1) return 1;
	if (res1 != 3) return 4;
	if (res2 != 2) return 5;
      ]]
    )],
    [ac_cv_func_snprintf="$func";break],
    [
    if test "$?$ac_cv_func_snprintf_noniso" = 1no ; then
      ac_cv_func_snprintf_noniso="$func"
    fi
    ac_cv_func_snprintf=no
    ],
    [ac_cv_func_snprintf=unknown;break]
  )
done
AC_MSG_RESULT([$ac_cv_func_snprintf])
case $ac_cv_func_snprintf in
  no)
    AC_MSG_CHECKING(for working non-ISO C90 conforming snprintf)
    AC_MSG_RESULT([$ac_cv_func_snprintf_noniso])
    if test no = "$ac_cv_func_snprintf_noniso" ; then
      AC_DEFINE_UNQUOTED(SNPRINTF, [$ac_cv_func_snprintf_noniso],
	[Define to the name of a function implementing snprintf but not caring about ISO C90 return value semantics (if one exists)])
    fi
    ;;
  unknown)
    dnl be conservative when crosscompiling
    ;;
  *)
    AC_DEFINE_UNQUOTED(SNPRINTF_ISO, [$ac_cv_func_snprintf],
       [Define to the name of a function implementing snprintf with ISO C90 semantics (if one exists)])
    AC_DEFINE_UNQUOTED(SNPRINTF, [$ac_cv_func_snprintf],
       [Define to the name of a function implementing snprintf but not caring about ISO C90 return value semantics (if one exists)])
    ;;
esac

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
