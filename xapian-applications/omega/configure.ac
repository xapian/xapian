dnl Process this file with autoconf to produce a configure script.

dnl Need autoconf 2.50 or later for many features
dnl automake 1.6.3 requires autoconf 2.54
dnl autoconf 2.59 is what xapian-core requires, so be consistent
AC_PREREQ(2.59)
AC_INIT(xapian-omega, 0.9.10)dnl FIXME: bugreport addr as third argument
dnl Need 1.5 for AM_CXXFLAGS, etc; 1.6.3 for AM_INIT_AUTOMAKE with no args
dnl 1.8.5 contains a lot of fixes over 1.6.3
dnl 1.9 reduces Makefile.in size, and 1.9.5 was in Debian sarge
AM_INIT_AUTOMAKE([1.9.5 -Wportability])

AC_CONFIG_SRCDIR(omega.cc)

AC_CONFIG_HEADERS(config.h)

dnl We don't use Fortran, but libtool 1.5 insists on probing for f77 which just
dnl makes configure take longer to run.  We force it not to using the method
dnl suggested here (which also reduces configure size):
dnl http://article.gmane.org/gmane.comp.gnu.libtool.general/8172
m4_defun([_LT_AC_LANG_F77_CONFIG], [:])
dnl Eliminating the unused code for gcj and rc probes further reduces the size
dnl of the generated configure script (by 29% in total):
m4_defun([_LT_AC_LANG_GCJ_CONFIG], [:])
m4_defun([_LT_AC_LANG_RC_CONFIG], [:])

dnl Check for libxapian
AC_PROG_LIBTOOL
XO_LIB_XAPIAN
dnl We want XAPIAN_CXXFLAGS to be used for configure tests
CXXFLAGS="$CXXFLAGS $XAPIAN_CXXFLAGS"

dnl disable "maintainer only" rules by default
AM_MAINTAINER_MODE

dnl We only need to set docdir for compatibility with autoconf < 2.60 - this
dnl code can be removed once we move to requiring autoconf 2.60 or newer.
test -n "$docdir" || docdir='${datadir}/doc/${PACKAGE_TARNAME}'
AC_SUBST(docdir)
 
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

dnl mingw (for instance) lacks ssize_t
AC_CHECK_TYPE(ssize_t, int)
AC_TYPE_MODE_T

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

dnl Add in portable replacement for mkdtemp on platforms which lack it.
AC_CHECK_FUNCS(mkdtemp)
AM_CONDITIONAL(NEED_MKDTEMP, [test yes != "$ac_cv_func_mkdtemp"])

dnl Need a full link check for posix_fadvise.
AC_LINK_IFELSE([
  AC_LANG_PROGRAM(
    [[
#ifdef __linux__
# define _POSIX_C_SOURCE 200112L /* for posix_fadvise from fcntl.h */
#endif
#include <fcntl.h>
    ]], [[
      posix_fadvise(1, 0, 0, POSIX_FADV_NOREUSE);
      posix_fadvise(1, 0, 0, POSIX_FADV_SEQUENTIAL);
      posix_fadvise(1, 0, 0, POSIX_FADV_DONTNEED);
    ]])],
  [AC_DEFINE(HAVE_POSIX_FADVISE, 1, [Define to 1 if you have the 'posix_fadvise' function])]
)

AC_CHECK_FUNCS(mmap)

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

AC_ARG_WITH(iconv,
  AC_HELP_STRING([--with-iconv], [force use of iconv (error if not found)])
  AC_HELP_STRING([--without-iconv], [don't use iconv]),
  [with_iconv=$withval], [with_iconv=auto])

if test no != "$with_iconv" ; then
  ICONV_LDFLAGS=
  AC_SEARCH_LIBS(iconv, iconv, [], [
    dnl For fink on OS X.
    if test -f "/sw/etc/fink.conf" ; then
      AC_CANONICAL_HOST
      case $host_os in
      darwin*)
	ICONV_LDFLAGS=-L/sw/lib
	SAVE_LDFLAGS=$LDFLAGS
	LDFLAGS="$LDFLAGS $ICONV_LDFLAGS"
	AC_CHECK_LIB(iconv, iconv, [], [
	  if test yes = "$with_iconv" ; then
	    AC_MSG_ERROR([iconv not found, but --with-iconv specified])
	  fi
	  ICONV_LDFLAGS=
	  with_iconv=no
	])
	LDFLAGS=$SAVE_LDFLAGS
      esac
    fi
  ])
  AC_SUBST(ICONV_LDFLAGS)
fi

if test no != "$with_iconv" ; then
  dnl Check if iconv take "char**" or "const char**" as its input.
  dnl Solaris 2.9 has "const char **"; GNU iconv has "char **".
  AC_MSG_CHECKING([for type of input pointer for iconv])
  AC_CACHE_VAL([xo_cv_iconv_input],
    [
      xo_cv_iconv_input=
      for t in 'char*' 'const char *' ; do
	dnl Use a link check as just compiling was falsely detecting a
	dnl working iconv installation on some platforms.
	AC_LINK_IFELSE(
	  [AC_LANG_PROGRAM(
	    [#include <iconv.h>],
	    [[
	      char buf[20];
	      $t in = "hello";
	      size_t in_len = 5;
	      char * out = buf;
	      size_t out_len = 20;
	      iconv((iconv_t)-1, &in, &in_len, &out, &out_len);
	    ]])],
	  [
	  xo_cv_iconv_input=$t
	  break
	])
      done
      if test -z "$xo_cv_iconv_input"; then
	if test yes = "$with_iconv" ; then
	  AC_MSG_RESULT([not found])
	  AC_MSG_ERROR([Failed to work out how to call iconv but --with-iconv specified])
	fi
	AC_MSG_RESULT([not found - not using iconv])
	with_iconv=no
      fi
    ])
  AC_MSG_RESULT([$xo_cv_iconv_input])
  AC_DEFINE_UNQUOTED(ICONV_INPUT_TYPE, [$xo_cv_iconv_input],
		     [type of input pointer for iconv])
fi

if test no != "$with_iconv" ; then
  AC_DEFINE(USE_ICONV, 1, [Define if iconv() should be used for converting character sets.])
fi

dnl Check processor endianness.
AC_C_BIGENDIAN

if test x$USE_MAINTAINER_MODE = xyes; then
  dnl Check for perl. (Needed to make man pages from "--help" output).
  AC_PATH_PROG(PERL, perl, [])
  test -z "$PERL" && AC_MSG_ERROR([perl is required in maintainer mode])

  dnl Check for help2man. (Needed to make man pages from "--help" output).
  AC_PATH_PROG(HELP2MAN, help2man, [])
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
fi

AC_SUBST(AM_CXXFLAGS)

AH_BOTTOM(
[/* Disable stupid MSVC "performance" warning for converting int to bool. */
#ifdef _MSC_VER
# pragma warning(disable:4800)
#endif])

AC_ARG_ENABLE(quiet,
  [AS_HELP_STRING([--enable-quiet], [Enable quiet building [default=no]])],
  [case ${enableval} in
    yes|no) ;;
    *) AC_MSG_ERROR(bad value ${enableval} for --enable-quiet) ;;
  esac])

dnl Pass `--quiet' to libtool if quiet building is requested.  Using an
dnl AC_SUBST-ed value like this allows the choice to be easily overridden
dnl if you want to see exactly what make and libtool are up to - just run
dnl make like this: `make QUIET='
QUIET=
if test yes = "$enable_quiet"; then
  QUIET=--quiet
fi
AC_SUBST(QUIET)

dnl **************************
dnl * Build the output files *
dnl **************************

AC_CONFIG_FILES([Makefile xapian-omega.spec])
AC_CONFIG_FILES([makemanpage], [chmod +x makemanpage])
AC_OUTPUT
