dnl Process this file with autoconf to produce a configure script.

dnl NB When updating the version for a release, update LIBRARY_VERSION_INFO
dnl below too.
m4_define([project_version], [1.3.0])
AC_INIT([xapian-letor], project_version, [https://xapian.org/bugs])

AC_PREREQ([2.64])
dnl Extract from the libtool info manual:
dnl
dnl Here are a set of rules to help you update your library version information:
dnl
dnl 1. Start with version information of '0:0:0' for each libtool library.
dnl
dnl 2. Update the version information only immediately before a public
dnl    release of your software.  More frequent updates are unnecessary,
dnl    and only guarantee that the current interface number gets larger
dnl    faster.
dnl
dnl 3. If the library source code has changed at all since the last
dnl    update, then increment REVISION ('C:R:A' becomes 'C:r+1:A').
dnl
dnl 4. If any interfaces have been added, removed, or changed since the
dnl    last update, increment CURRENT, and set REVISION to 0.
dnl
dnl 5. If any interfaces have been added since the last public release,
dnl    then increment AGE.
dnl
dnl 6. If any interfaces have been removed since the last public release
dnl    then set AGE to 0.
dnl
dnl LIBRARY_VERSION_INFO for libxapianletor:
dnl 0:0:0 1.3.1 First release with xapian-letor split out from xapian-core
LIBRARY_VERSION_INFO=0:0:0
AC_SUBST(LIBRARY_VERSION_INFO)

dnl Where xapian-letor.h, etc go.  In development release append "/xapian-1.3".
incdir=$includedir
AC_SUBST([incdir])

AM_INIT_AUTOMAKE([1.10.1 -Wportability tar-ustar])

AC_CONFIG_SRCDIR([ranker/ranker.cc])

AC_CONFIG_HEADERS([config.h])

AC_CONFIG_MACRO_DIR([m4])

LT_PREREQ([2.2.8])
LT_INIT
dnl -no-undefined causes problems on Solaris with Sun CC in C++11 mode, so only
dnl pass -no-undefined on platforms where it is required in order to link a
dnl shared library at all (Windows is the main one).
NO_UNDEFINED=
if test unsupported = "$allow_undefined_flag" ; then
  NO_UNDEFINED=-no-undefined
fi
AC_SUBST(NO_UNDEFINED)


dnl disable "maintainer only" rules by default
AM_MAINTAINER_MODE

dnl Checks for programs.
AC_PROG_CXX

AC_CANONICAL_HOST

# Checked: freebsd8.0 openbsd4.6 solaris2.9 solaris2.10
case $host_os in
  linux* | k*bsd*-gnu | freebsd* | openbsd* | solaris*)
    dnl Vanilla libtool sets this to "unknown" which it then handles as "yes".
    link_all_deplibs_CXX=no
    ;;
esac

case $host_os in
  linux*)
    dnl Extract search path from ldconfig which is more reliable than the way
    dnl vanilla libtool extracts them from ld.so.conf.
    d=`/sbin/ldconfig -N -X -v 2>&AS_MESSAGE_LOG_FD|$SED 's,^\(/.*\):\( (.*)\)\?$,\1,p;d'|tr '\n' ' '`
    test -z "$d" || sys_lib_dlsearch_path_spec=$d
    ;;
esac

case $host in
  *-*-cygwin* | *-*-mingw* | *-*-pw32* | *-*-os2* | *-*-darwin* | *-cegcc*)
    dnl On these platforms, libtool emits a warning if -no-install is passed,
    dnl and uses -no-fast-install instead - the least ugly way to avoid that
    dnl warnings seems to be to duplicate the above list of platforms from
    dnl libtool and use -no-fast-install on them directly.
    NO_INSTALL=-no-fast-install ;;
  *)
    NO_INSTALL=-no-install ;;
esac
AC_SUBST([NO_INSTALL])

dnl Probe for any options needed to enable C++11 support.
AX_CXX_COMPILE_STDCXX_11

dnl Run tests using the C++ compiler.
AC_LANG_CPLUSPLUS

dnl Check for xapian-core.
XO_LIB_XAPIAN([], [],
    [xapian-config]regexp(project_version,
			  [^\([0-9]*\.[0-9]*[13579]\)\..*$], [-\1]))
XO_REQUIRE([1.4.0])

dnl We want XAPIAN_CXXFLAGS to be used for configure tests.
save_CXXFLAGS=$CXXFLAGS
CXXFLAGS="$CXXFLAGS $XAPIAN_CXXFLAGS"
dnl mingw (for instance) lacks ssize_t
AC_TYPE_SSIZE_T

AC_TYPE_PID_T

AC_TYPE_MODE_T

dnl Check for perl (needed to generate some sources and documentation).
AC_PATH_PROG([PERL], [perl], [])
if test x$USE_MAINTAINER_MODE = xyes; then
  test -z "$PERL" && AC_MSG_ERROR([perl is required in maintainer mode])
fi

AC_ARG_ENABLE([documentation],
  [AS_HELP_STRING([--enable-documentation], [enable make rules to rebuild documentation [default=maintainer-mode]])],
  [case ${enableval} in
    yes|no) ;;
    *) AC_MSG_ERROR([bad value ${enableval} for --enable-documentation]) ;;
  esac],
  [enable_documentation=$USE_MAINTAINER_MODE])
AM_CONDITIONAL([DOCUMENTATION_RULES], [test x"$enable_documentation" = xyes])
AM_CONDITIONAL([MAINTAINER_NO_DOCS], [test x"$USE_MAINTAINER_MODE$enable_documentation" = xyesno])

if test x"$enable_documentation" = xyes ; then
  dnl Check for help2man. (Needed to make man pages from "--help" output).
  AC_PATH_PROG([HELP2MAN], [help2man], [])
  test -z "$HELP2MAN" && AC_MSG_ERROR([help2man is required to build documentation])

  dnl Check for rst2html. (Needed to make HTML from reStructuredText format)
  dnl Also look for rst2html.py, which archlinux reportedly installs it as.
  AC_PATH_PROGS([RST2HTML], [rst2html rst2html.py], [])
  test -z "$RST2HTML" && AC_MSG_ERROR([rst2html is required to build documentation (try package python-docutils)])
fi

dnl Checks for header files.
AC_CHECK_HEADERS([fcntl.h limits.h sys/errno.h sys/select.h], [], [], [ ])

dnl C++11 should have log2(), but keep this check for now to allow for
dnl compilers with incomplete library support for C++11.
AC_CHECK_DECLS([log2], [], [], [#include <cmath>])

dnl libsvm is required.
AC_CHECK_HEADER([libsvm/svm.h], [], AC_MSG_ERROR([libsvm required but libsvm/svm.h not found]), [ ])
save_LIBS=$LIBS
LIBS=
AC_SEARCH_LIBS([svm_train], [svm], [], AC_MSG_ERROR([libsvm required]))
LIBSVM_LIBS=$LIBS
LIBS=$save_LIBS
AC_SUBST([LIBSVM_LIBS])

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
AC_MSG_CHECKING([for working ISO C90 conforming snprintf])
ac_cv_func_snprintf_noniso=no
for func in snprintf _snprintf ; do
  AC_RUN_IFELSE([
    AC_LANG_PROGRAM(
      [[
#include <cstdio>
#include <cstring>
using namespace std;
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
    AC_MSG_CHECKING([for working non-ISO C90 conforming snprintf])
    AC_MSG_RESULT([$ac_cv_func_snprintf_noniso])
    if test no != "$ac_cv_func_snprintf_noniso" ; then
      AC_DEFINE_UNQUOTED([SNPRINTF], [$ac_cv_func_snprintf_noniso],
	[Define to the name of a function implementing snprintf but not caring about ISO C99 return value semantics (if one exists)])
    fi
    ;;
  unknown)
    dnl be conservative when crosscompiling
    ;;
  *)
    AC_DEFINE_UNQUOTED([SNPRINTF_ISO], [$ac_cv_func_snprintf],
       [Define to the name of a function implementing snprintf with ISO C99 semantics (if one exists)])
    AC_DEFINE_UNQUOTED([SNPRINTF], [$ac_cv_func_snprintf],
       [Define to the name of a function implementing snprintf but not caring about ISO C99 return value semantics (if one exists)])
    ;;
esac

dnl ******************************
dnl * Set special compiler flags *
dnl ******************************

dnl Set flags to control warnings (enable more, or disable annoying ones).
dash_d_visibility=
if test yes = "$GXX"; then
  dnl Intel's C++ compiler and clang both lie and define __GNUC__, so check which
  dnl we actually have, as neither is really 100% compatible.
  case `echo __INTEL_COMPILER __clang__|$CXX -E - 2>&AS_MESSAGE_LOG_FD|grep -v '^#'` in
    *__INTEL_COMPILER*__clang__*)
      dnl GCC (since neither substituted):
      dnl
      dnl -Wundef was supported by g++ 3.0.
      dnl
      dnl -Wlogical-op and -Wmissing-declarations (for C++) were added in
      dnl GCC 4.3.
      dnl
      dnl All the other options were supported by g++ 2.95.
      AM_CXXFLAGS="$AM_CXXFLAGS -Wall -W -Wredundant-decls -Wpointer-arith -Wcast-qual -Wcast-align -Wformat-security -fno-gnu-keywords -Wundef -Woverloaded-virtual -Wstrict-null-sentinel -Wshadow -Wstrict-overflow=1 -Wlogical-op -Wmissing-declarations"

      dnl The exact format of g++ --version seems to change with almost every
      dnl minor release so use the preprocessor macros which should be more
      dnl robust.
      AC_MSG_CHECKING([for version of $CXX])
      gxx_version=`echo __GNUC__.__GNUC_MINOR__.__GNUC_PATCHLEVEL__|$CXX -E -|sed '/^#/d;s/ //g'`
      AC_MSG_RESULT([GCC $gxx_version])

      case $gxx_version in
      [[0123]].*|4.[[0123]].*)
	AC_MSG_ERROR([Xapian requires GCC 4.4 or later])
	;;
      4.[[45]].*) ;;
      *) dnl GCC >= 4.6
	dnl -Wdouble-promotion was added in GCC 4.6.
	dnl
	dnl -Winit-self was added in GCC 3.4, but for GCC < 4.7 g++ always
	dnl warns for this case with -Wuninitialized (implied by -W).  We
	dnl don't intend to use this idiom, so any instances are bugs we'd
	dnl like to know about.
	AM_CXXFLAGS="$AM_CXXFLAGS -Wdouble-promotion -Winit-self" ;;
      esac

      dnl FIXME:
      dnl -Wconversion in older GCC versions is intended to help migration
      dnl from K&R to ISO C, and isn't useful for us.  In 4.3 it was renamed
      dnl to -Wtraditional-conversion and a new -Wconversion added which
      dnl sounds useful but is a bit buggy currently.  So we should consider
      dnl enabling -Wconversion once it is stabilised (GCC 4.4 or ...)
      dnl
      dnl -Wold-style-cast is interesting, but triggers for macros from
      dnl system headers (e.g. FD_SET) (tested with GCC 4.4).

      dnl Automatically add -Werror if maintainer mode is enabled.
      if test x$USE_MAINTAINER_MODE = xyes; then
	AM_CXXFLAGS="$AM_CXXFLAGS -Werror"
      fi
      ;;
    *__clang__*)
      dnl Intel's compiler (since __clang__ not substituted):
      dnl
      dnl -w1 stops the avalanche of uninteresting "remark" messages.
      dnl -wd... disables warnings which don't have good code workarounds.
      AM_CXXFLAGS="$AM_CXXFLAGS -Wall -w1 -wd177,1572"

      dnl Automatically add -Werror if maintainer mode is enabled.
      if test x$USE_MAINTAINER_MODE = xyes; then
	AM_CXXFLAGS="$AM_CXXFLAGS -Werror"
      fi
      ;;
    *__INTEL_COMPILER*)
      dnl clang (since __INTEL_COMPILER not substituted):
      AM_CXXFLAGS="$AM_CXXFLAGS -Wall -W -Wredundant-decls -Wpointer-arith -Wcast-qual -Wcast-align -Wformat-security -fno-gnu-keywords -Wundef -Woverloaded-virtual"
      AM_CXXFLAGS="$AM_CXXFLAGS -Wshadow -Wstrict-overflow=1 -Winit-self -Wmissing-declarations"

      dnl Automatically add -Werror if maintainer mode is enabled.
      if test x$USE_MAINTAINER_MODE = xyes; then
	AM_CXXFLAGS="$AM_CXXFLAGS -Werror"
      fi
      ;;
  esac

  if test no != "$enable_visibility"; then
    dnl GCC doesn't support symbol visibility on all platforms (e.g. it isn't
    dnl on mingw).
    AC_MSG_CHECKING([if $CXX -fvisibility=hidden works])
    if echo 'int bar() __attribute__((visibility("default"))); int foo() {return 42;}'|$CXX -Werror -fvisibility=hidden -c -oconftest.o -xc++ - >&AS_MESSAGE_LOG_FD 2>&AS_MESSAGE_LOG_FD ; then
      AC_MSG_RESULT([yes])
      AM_CXXFLAGS="$AM_CXXFLAGS -fvisibility=hidden"
      dash_d_visibility=-DXAPIAN_ENABLE_VISIBILITY
    else
      AC_MSG_RESULT([no])
    fi
  fi
else
  dnl Not GCC, nor a compiler masquerading as GCC.
  case /$CXX in
  */aCC)
    dnl +w turns on more warnings.
    dnl +wlint turns on "lint-like" warnings.
    dnl +W<n1>,<n2>,... suppresses warnings n1, n2, ...
    dnl 2340 (remark) "value copied to temporary, reference to temporary
    dnl	use", in: throw Xapian::UnimplementedError("...");
    dnl 2401 "destructor for base class ... is non-virtual" (we don't need a
    dnl	virtual destructor for intrusive_base, since we never delete
    dnl       its subclasses by a intrusive_base *).
    dnl 3348 "declaration hides constant ..." which seems to misfire!
    dnl 4255 (remark) "padding size of struct "..." with ... bytes to
    dnl	alignment boundary".
    dnl 4273 "floating-point equality and inequality comparisons may be
    dnl	inappropriate due to roundoff common in floating-point computation"
    dnl	No obvious workaround for when you really do want == or !=.
    dnl 4285 "operator= does not have a check for the source and destination
    dnl	addresses being non-identical" - fires for AutoPtr which
    dnl	includes such a check indirectly (internaltest's autoptr1 check this).
    dnl 20201 "Memory leak is detected" which triggers for "return new Foo;"!
    AM_CXXFLAGS="$AM_CXXFLAGS +w +wlint +W2340,2401,3348,4255,4273,4285,20201" ;;
  esac
fi

AH_BOTTOM(
[/* Disable stupid MSVC "performance" warning for converting int to bool. */
#ifdef _MSC_VER
# pragma warning(disable:4800)
#endif

/* _FORTIFY_SOURCE is only supported by GCC >= 4.1 and glibc >= 2.3.4, but it
 * shouldn't cause a problem to define it where it's not supported and some
 * distros may have backported support, so hardcoding version checks is
 * counter-productive.
 *
 * Check if _FORTIFY_SOURCE is already defined to allow the user to override
 * our choice with "./configure CPPFLAGS=-D_FORTIFY_SOURCE=0" or "...=1".
 */
#if defined __GNUC__ && !defined _FORTIFY_SOURCE
# define _FORTIFY_SOURCE 2
#endif

/* For GCC >= 3.0 (and Intel's C++ compiler, which also defines __GNUC__),
 * we can use __builtin_expect to give the compiler hints about branch
 * prediction.  See HACKING for how to use these.
 */
#if defined __GNUC__
/* The arguments of __builtin_expect() are both long, so use !! to ensure that
 * the first argument is always an integer expression, and always 0 or 1, but
 * still has the same truth value for the if or while it is used in.
 */
# define rare(COND) __builtin_expect(!!(COND), 0)
# define usual(COND) __builtin_expect(!!(COND), 1)
#else
# define rare(COND) (COND)
# define usual(COND) (COND)
#endif
])

AC_DEFINE([XAPIAN_LIB_BUILD], [1], [Workaround for avoiding sub-header included error])

AC_SUBST([AM_CXXFLAGS])

dnl Restore CXXFLAGS to those the user specified or autoconf defaulted to.
CXXFLAGS=$save_CXXFLAGS

dnl Enable large file support if possible.
AC_SYS_LARGEFILE

dnl Required for auto regeneration to work in a combined maintainer-mode tree.
: ${AUTOM4TE=autom4te}
AC_SUBST([AUTOM4TE])

dnl We want to be able to use GNU make % pattern rules in maintainer targets
dnl but automake warns these aren't portable, so we substitute the % to avoid
dnl this warning.
PERCENT='%'
AC_SUBST([PERCENT])

dnl If valgrind is installed and new enough, we use it for leak checking in the
dnl testsuite.  If VALGRIND is set to an empty value, then skip the check and
dnl don't use valgrind.
if test -n "${VALGRIND-unset}" ; then
  AC_PATH_PROG([VALGRIND], [valgrind], [])
  if test -n "$VALGRIND" ; then
    dnl Check that the installed valgrind version works, and supports the
    dnl options we use.  This means we won't try to use valgrind < 3.3.0
    dnl (released 7/12/2007) since before that --log-file didn't expand
    dnl %p (and appended the process id).
    dnl
    dnl No need to check for VALGRIND_COUNT_LEAKS now - that was added before
    dnl 2.0.0.
    AC_MSG_CHECKING([if valgrind supports --log-file with %p])
    vglog=config.vglog.%p.tmp
    vglogwild="config.vglog.*.tmp*"
    rm -f $vglogwild
    if $VALGRIND --log-file="$vglog" -q true 2>&AS_MESSAGE_LOG_FD ; then
      for f in $vglogwild ; do
        case $f in
    $vglog*) VALGRIND= ;;
    esac
      done
      if test x"$VALGRIND" = x ; then
    AC_MSG_RESULT([no])
      else
    AC_MSG_RESULT([yes])
    AC_CHECK_HEADERS([valgrind/memcheck.h], [], [VALGRIND=], [ ])
      fi
    else
      dnl The valgrind detected doesn't seem to work!  Perhaps this is an
      dnl x86_64 box with a 32 bit valgrind.
      AC_MSG_RESULT([$VALGRIND doesn't work])
      VALGRIND=
    fi
    rm -f $vglogwild
  fi
fi

if test -n "$VALGRIND" ; then
  AC_DEFINE([HAVE_VALGRIND], [1], [Define if a suitable valgrind is installed])
fi

REMOTE_LIBS=
if true ; then
  AC_PREPROC_IFELSE([AC_LANG_SOURCE([[
#ifdef __WIN32__
#error WIN32
#endif
  ]])], [win32=no], [win32=yes])

  case $host_os-$win32 in
    *-yes )
      dnl For mingw and msvc we have an alternative implementation which
      dnl doesn't need fork() or socketpair().
      dnl
      dnl We need -lws2_32 for getaddrinfo(), etc.
      REMOTE_LIBS=-lws2_32
      dnl Vista is needed for the AI_ADDRCONFIG flag to getaddrinfo().
      AC_DEFINE([WINVER], [0x600],
		[Version of Windows to assume (0x600 => Vista).])
      AC_DEFINE([_WIN32_WINNT], [WINVER],
		[Version of Windows to assume.])
      ;;
    *djgpp* | *msdos* )
      dnl DJGPP has a dummy implementation of fork which always fails.
      dnl
      dnl For disk-based backend, use flock() for locking, which doesn't need
      dnl fork() or socketpair().
      AC_DEFINE([FLINTLOCK_USE_FLOCK], 1, [Define to use flock() for flint-compatible locking])
      dnl If someone actually wanted remote backend support, then DJGPP has a
      dnl pthreads port, so using threads like we do on Windows would make more
      dnl sense.
      enable_backend_remote=no
      ;;
    *)
      dnl On Unix, we need fork and socketpair for the remotebackend.
      SAVE_LIBS=$LIBS
      AC_CHECK_FUNCS([fork], [], [
	AC_MSG_ERROR([fork() required for the remote backend - if an extra library is needed, pass LIBS=-lfoo to configure.  Or --disable-backend-remote to disable it.)])
      ])
      dnl Check if -lsocket is required for socketpair (Solaris needs it).
      dnl And on Haiku it's in -lnetwork.
      AC_SEARCH_LIBS([socketpair], [socket network], [], [
	AC_MSG_ERROR([socketpair() required for the remote backend - if an extra library is needed, pass LIBS=-lfoo to configure.  Or --disable-backend-remote to disable it.)])
      ])
      AC_DEFINE([HAVE_SOCKETPAIR], [1],
		[Define to 1 if you have the 'socketpair' function.])
      dnl Check if extra libraries are needed for getaddrinfo or inet_ntop()
      dnl (e.g. on Solaris).
      dnl
      dnl We're currently assuming that any system that is worth trying to
      dnl support has getaddrinfo() and inet_ntop(), since these are the
      dnl standard route for supporting IPv6, and that's pretty much essential
      dnl for platforms to support now.
      AC_SEARCH_LIBS([getaddrinfo], [nsl socket], [], [
	AC_MSG_ERROR([getaddrinfo() required for the remote backend - if an extra library is needed, pass LIBS=-lfoo to configure.  Or --disable-backend-remote to disable it.)])
      ])
      AC_SEARCH_LIBS([inet_ntop], [nsl socket], [], [
	AC_MSG_ERROR([inet_ntop() required for the remote backend - if an extra library is needed, pass LIBS=-lfoo to configure.  Or --disable-backend-remote to disable it.)])
      ])
      REMOTE_LIBS=$LIBS
      LIBS=$SAVE_LIBS
      ;;
  esac

  if test "$enable_backend_remote" = yes ; then
    TYPE_SOCKLEN_T
    XAPIAN_LIBS="$XAPIAN_LIBS $REMOTE_LIBS"
  fi
fi

dnl If eatmydata is installed, we run the testsuite under it to speed it up.
dnl If EATMYDATA is set to an empty value, then skip this check and don't use
dnl eatmydata.
if test -n "${EATMYDATA-unset}" ; then
  AC_PATH_PROG([EATMYDATA], [eatmydata], [])
fi

dnl Used by tests/harness/testsuite.cc
AC_CHECK_FUNCS([sigaction])
AC_MSG_CHECKING([for sigsetjmp and siglongjmp])
AC_TRY_COMPILE([#include <setjmp.h>],
  [sigjmp_buf jb; if (sigsetjmp(jb, 1)) { siglongjmp(jb, 1);  }],
  AC_DEFINE([HAVE_SIGSETJMP], [1], [Define to 1 if you have the 'sigsetjmp' function])
  AC_MSG_RESULT([yes]),
  AC_MSG_RESULT([no]))

AC_ARG_ENABLE([visibility],
  [AS_HELP_STRING([--disable-visibility], [disable use of GCC visibility])],
  [case ${enableval} in
    yes|no) ;;
    *) AC_MSG_ERROR([bad value ${enableval} for --disable-visibility]) ;;
  esac])

vpath_build=no
if test "`pwd`" != "`cd $srcdir;pwd`" ; then
  vpath_build=yes
fi
AM_CONDITIONAL([VPATH_BUILD], [test yes = "$vpath_build"])

dnl Decide if we should use the zlib-vg.so LD_PRELOAD hack.
use_zlib_vg=no
if test -n "$VALGRIND" ; then
  case $host_os in
    linux*) use_zlib_vg=yes ;;
  esac
fi
AM_CONDITIONAL([USE_ZLIB_VG], [test yes = "$use_zlib_vg"])

dnl **************************
dnl * Build the output files *
dnl **************************

AC_CONFIG_FILES([
 Makefile
 tests/Makefile
 docs/Makefile
 ])
AC_CONFIG_FILES([makemanpage], [chmod +x makemanpage])
AC_CONFIG_FILES([tests/runtest], [chmod +x tests/runtest])
AC_OUTPUT
