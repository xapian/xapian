dnl Process this file with autoconf to produce a configure script.

dnl Need autoconf 2.50 or later for many features
dnl automake 1.6.3 requires autoconf 2.54
dnl autoconf 2.59 is what xapian-core requires, so be consistent
AC_PREREQ(2.59)
AC_INIT(xapian-omega, 0.9.9)dnl FIXME: bugreport addr as third argument
# Need 1.5 for AM_CXXFLAGS, etc; 1.6.3 for AM_INIT_AUTOMAKE with no args
# 1.8.5 contains a lot of fixes over 1.6.3
AM_INIT_AUTOMAKE([1.8.5])

AC_CONFIG_SRCDIR(omega.cc)

AC_CONFIG_HEADERS(config.h)

dnl Check for libxapian
AC_PROG_LIBTOOL
XO_LIB_XAPIAN
dnl We want XAPIAN_CXXFLAGS to be used for configure tests
CXXFLAGS="$CXXFLAGS $XAPIAN_CXXFLAGS"

dnl disable "maintainer only" rules by default
AM_MAINTAINER_MODE

dnl Checks for programs.
AC_PROG_CXX

dnl Run tests using the C++ compiler.
AC_LANG_CPLUSPLUS

dnl IRIX helpfully won't allow stdint.h to be included from C++ code,
dnl so we can't just use AC_CHECK_HEADERS.
AC_TRY_COMPILE([#include <stdint.h>],
  [uint32_t foo = 7; return (int)foo;],
  AC_DEFINE(HAVE_WORKING_STDINT_H, 1, [Define to 1 if you have the <stdint.h> header file and it can be used in C++ code.]))

dnl Check for headers.
AC_CHECK_HEADERS([netinet/in.h arpa/inet.h sys/wait.h strings.h],
		 [], [], [ ])

dnl Check for time functions.
AC_FUNC_STRFTIME
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
dnl
dnl mingw has _snprintf so check for that too.
AC_MSG_CHECKING(for working ISO C90 conforming snprintf)
ac_cv_func_snprintf_noniso=no
for func in snprintf _snprintf ; do
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
	int res1 = $func(buffer, 2, "%s", "XYZ");
	if (memcmp(buffer, "X\0c", 4) != 0) return 2;
	int res2 = $func(buffer, 2, "%x", 0x12);
	if (memcmp(buffer, "1\0c", 4) != 0) return 3;
	if (res1 == -1 && res2 == -1) return 15; /* Pre-ISO semantics. */
	if (res1 != 3) return 4;
	if (res2 != 2) return 5;
      ]]
    )],
    [ac_cv_func_snprintf=$func;break],
    [
    if test 15no = "$?$ac_cv_func_snprintf_noniso" ; then
      ac_cv_func_snprintf_noniso=$func
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
    if test no != "$ac_cv_func_snprintf_noniso" ; then
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

dnl Check if iconv take "char**" or "const char**" as its input.
dnl Solaris 2.9 has "const char **"; GNU iconv has "char **".
AC_MSG_CHECKING([for type of input pointer for iconv])
AC_CACHE_VAL([xo_cv_iconv_input],
  [
    for t in 'char*' 'const char *' ; do
      AC_TRY_COMPILE([
#include <iconv.h>
      ],[
	char buf[20];
	$t in = "hello";
	size_t in_len = 5;
	char * out = buf;
	size_t out_len = 20;
	iconv((iconv_t)-1, &in, &in_len, &out, &out_len);
      ],[
	xo_cv_iconv_input="$t"
	break
      ])
    done
    if test -z "$xo_cv_iconv_input"; then
      AC_MSG_RESULT([not found])
      AC_MSG_ERROR([Failed to work out how to call iconv])
    fi
  ])
AC_MSG_RESULT([$xo_cv_iconv_input])
AC_DEFINE_UNQUOTED(ICONV_INPUT_TYPE, [$xo_cv_iconv_input],
		   [type of input pointer for iconv])

ICONV_LDFLAGS=
AC_SEARCH_LIBS(iconv, iconv, [], [
  dnl For fink on OS X.
  if test -f "/sw/etc/fink.conf" ; then
    AC_CANONICAL_HOST
    case $host_os in
    darwin*) ICONV_LDFLAGS=-L/sw/lib ;;
    esac
  fi
  SAVE_LDFLAGS=$LDFLAGS
  ICONV_LDFLAGS=-L/sw/lib
  LDFLAGS="$LDFLAGS $ICONV_LDFLAGS"
  AC_CHECK_LIB(iconv, iconv, [], [
    AC_MSG_ERROR([Omega requires iconv to build])
  ])
  LDFLAGS=$SAVE_LDFLAGS
])
AC_SUBST(ICONV_LDFLAGS)

dnl Check processor endianness.
AC_C_BIGENDIAN

dnl Check for help2man. (Needed to make man pages from "--help" output).
AC_PATH_PROG(HELP2MAN, help2man, [])
if test x$USE_MAINTAINER_MODE = xyes; then
  test -z "$HELP2MAN" && AC_MSG_ERROR([help2man is required in maintainer mode])
fi

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

if test yes = "$GXX"; then
  dnl Intel's C++ compiler is identified as "GXX" by autoconf's test - check
  dnl which we actually have.
  AC_EGREP_CPP(yes,
    [#ifdef __INTEL_COMPILER
     yes
     #endif
    ],
    [
      dnl Intel's compiler:
      dnl
      dnl -w1 stops the avalanche of uninteresting "remark" messages.
      dnl -wd... disables warnings which don't have good code workarounds.
      AM_CXXFLAGS="$AM_CXXFLAGS -Wall -w1 -wd177,1572"
      dnl Automatically add -Werror if maintainer mode is enabled.
      if test x$USE_MAINTAINER_MODE = xyes; then
	AM_CXXFLAGS="$AM_CXXFLAGS -Werror"
      fi
    ],
    [
      dnl GCC:
      dnl
      dnl All these options were supported by g++ 2.95 and there's little
      dnl likelihood Xapian will build with any earlier version, so there's
      dnl not much point worrying about whether older versions had them or not.
      AM_CXXFLAGS="$AM_CXXFLAGS -Wall -W -Wredundant-decls -Wpointer-arith -Wcast-qual -Wcast-align -Wno-multichar -Wno-long-long -fno-gnu-keywords"

      dnl GCC2 reports e.g. "2.7.2.3" or "2.95.4".  3.0 reports e.g. "3.0.4"
      dnl but somewhere in the 3.X series the output became more verbose
      dnl (3.2 has the new style output - not sure about 3.1).  So we test
      dnl for older version numbers where possible.
      gxx_version=`$CXX --version 2>&1`

      case $gxx_version in
      2.96) dnl Oddly Redhat's "2.96" doesn't support -Wundef, though real
	    dnl GCC versions before and after it do!
        ;;
      [[12]].*) dnl GCC < 3
        AM_CXXFLAGS="$AM_CXXFLAGS -Wundef" ;;
      3.0*) dnl -Wshadow spews false positives with GCC 3.0.4
        AM_CXXFLAGS="$AM_CXXFLAGS -Wundef" ;;
      *)
	AM_CXXFLAGS="$AM_CXXFLAGS -Wundef -Wshadow" ;;
      esac

      dnl Automatically add -Werror if maintainer mode is enabled and we're
      dnl using GCC3 or newer.  We don't do this for older GCCs as GCC 2.95
      dnl issues spurious warnings.
      if test x$USE_MAINTAINER_MODE = xyes; then
	case $gxx_version in
	[[12]].*) ;; dnl GCC < 3
	*) AM_CXXFLAGS="$AM_CXXFLAGS -Werror" ;;
	esac
      fi
    ])
else
  dnl With SGI's C++ compiler, we have to specify "-ptused" or we get strange
  dnl template linking errors.
  AC_TRY_COMPILE([],
    [#ifndef _SGI_COMPILER_VERSION
  choke me
#endif],
    [AM_CXXFLAGS="$AM_CXXFLAGS -O2 -ptused"])
fi

AC_SUBST(AM_CXXFLAGS)

AH_BOTTOM(
[/* Disable stupid MSVC "performance" warning for converting int to bool. */
#ifdef _MSC_VER
# pragma warning(disable:4800)
#endif])

dnl **************************
dnl * Build the output files *
dnl **************************

AC_CONFIG_FILES([Makefile omega.spec])
AC_OUTPUT
