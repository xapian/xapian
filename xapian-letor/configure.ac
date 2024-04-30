dnl Process this file with autoconf to produce a configure script.

dnl NB When updating the version for a release, update LIBRARY_VERSION_INFO
dnl below too.
m4_define([project_version], [1.5.0])
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

dnl Libtool sets this (to yes|no|unknown) and we use it in xapianletor-config.
AC_SUBST([link_all_deplibs_CXX])

AM_INIT_AUTOMAKE([1.13 -Wportability tar-ustar no-dist-gzip dist-xz std-options])

AC_CONFIG_SRCDIR([ranker/ranker.cc])

AC_CONFIG_HEADERS([config.h])

AC_CONFIG_MACRO_DIRS([m4])

AM_PROG_AR

dnl Use libtool to manage our libraries.
LT_PREREQ([2.2.8])
dnl Default to only building shared libraries.
dnl
dnl Building both shared and static means having to compile the files which
dnl make up the library twice on most platforms.  Shared libraries are the
dnl better option for most users, and if anyone really wants static libraries,
dnl they can configure with --enable-static (or --enable-static=xapian-core if
dnl configuring a combined tree with the bindings).
dnl
dnl We don't export any data items from the library, so it should be safe to
dnl enable win32-dll without decorating any declarations specially.
LT_INIT([disable-static win32-dll])

dnl Only pass -no-undefined on platforms where it is required in order to link
dnl a shared library at all (Microsoft Windows is the main one) as it can cause
dnl problems on other platforms (e.g. Solaris with Sun CC in C++11 mode, though
dnl we don't support that compiler now as it lacks C++17 support).
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

# Checked: dragonfly6.4 freebsd8.0 netbsd9.3 openbsd4.6 solaris2.9 solaris2.10
case $host_os in
  linux* | k*bsd*-gnu | dragonfly* | freebsd* | netbsd* | openbsd* | solaris*)
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

dnl Probe for any options needed to enable C++17 support.
AX_CXX_COMPILE_STDCXX([17])

dnl We don't use a C compiler to compile Xapian's code, but on some platforms
dnl (e.g. mingw) libtool uses $LTCC which defaults to $CC, and it's also use to
dnl build auxiliary tools like snowball and lemon.
AC_PROG_CC

dnl Check endianness.
AC_C_BIGENDIAN

dnl Run tests using the C++ compiler.
AC_LANG([C++])

dnl Check for xapian-core.
XO_LIB_XAPIAN([], [],
    [xapian-config]regexp(project_version,
			  [^\([0-9]*\.[0-9]*[13579]\)\..*$], [-\1]))
XO_REQUIRE([1.4.0])

dnl Enable large file support if possible.
AC_SYS_LARGEFILE
dnl With xlC on AIX, -D_LARGE_FILES changes the ABI of std::string, so it
dnl also needs to be used when compiling user code.
abi_affecting_cxxflags=
if $GREP '^#define _LARGE_FILES 1$' confdefs.h > /dev/null 2>&1 ; then
  abi_affecting_cxxflags=-D_LARGE_FILES
fi
AC_SUBST([abi_affecting_cxxflags])

XAPIANLETOR_LDFLAGS=
AC_SUBST([XAPIANLETOR_LDFLAGS])

AC_PREPROC_IFELSE([AC_LANG_SOURCE([[
#if defined __WIN32__ || defined _WIN32
#error WIN32
#endif
]])], [win32=no], [win32=yes])

xo_fn_unknown_option_check() {
  dnl Some compilers don't error out on unknown options which unhelpfully
  dnl means we would think the option is supported, use it, and end up
  dnl with a warning or note diagnostic every time we use the option.
  dnl
  dnl Check compiler stderr for known strings:
  dnl * MSVC: Command line warning D9002 : ignoring unknown option '...'
  dnl * MSVC: LINK : warning LNK4044: unrecognized option '...'; ignored
  dnl * icc strings cribbed from automake's depend.m4
  $GREP 'ignoring unknown option' conftest.err >/dev/null 2>&1 ||
      $GREP 'unrecognized option' conftest.err >/dev/null 2>&1 ||
      $GREP 'ignoring option' conftest.err >/dev/null 2>&1 ||
      $GREP 'not supported' conftest.err >/dev/null 2>&1 ||
      return 1
  return 0
}

AC_DEFUN([XAPIAN_TEST_FLAGS_],
  [
  m4_define([cachevar],
	    [xo_cv_]m4_tolower(m4_bpatsubst([$1][_$3], [[^A-Za-z0-9]], [_])))
  AC_CACHE_CHECK([for $CXX with $3 in $1], cachevar,
    [
    XTF_save_$1=${$1}
    $1="${$1} $3"
    $2([AC_LANG_PROGRAM([], [])],
      [if xo_fn_unknown_option_check ; then
	 cachevar=no
       else
	 cachevar=yes
       fi],
      [cachevar=no])
    $1=$XTF_save_$1
    if test "$cachevar" = yes ; then
      $4="${$4} $3"
      $5
    else :
      $6
    fi
    ])
  ])

dnl Test if compiling works with $1 added to CXXFLAGS; if it does, add $1 to
dnl variable $2.  If the test passes, also do $3; if it fails, also do $4.
AC_DEFUN([XAPIAN_TEST_CXXFLAGS],
  [XAPIAN_TEST_FLAGS_([CXXFLAGS], [AC_COMPILE_IFELSE], [$1], [$2], [$3], [$4])])

dnl Test if linking works with $1 added to LDFLAGS; if it does, add $1 to
dnl variable $2.  If the test passes, also do $3; if it fails, also do $4.
AC_DEFUN([XAPIAN_TEST_LDFLAGS],
  [XAPIAN_TEST_FLAGS_([LDFLAGS], [AC_LINK_IFELSE], [$1], [$2], [$3], [$4])])

dnl Preserve the default CXXFLAGS.
save_CXXFLAGS=$CXXFLAGS

dnl x86 has excess precision issues with 387 FP instructions, which are
dnl avoided by using SSE instructions instead.  This is also faster (~6% in
dnl a CPU bound testcase).
AC_ARG_ENABLE([sse],
[AS_HELP_STRING([--disable-sse],
		[disable use of SSE FP instructions on x86])]
[AS_HELP_STRING([[--enable-sse[=sse|sse2]]],
		[set which SSE FP instructions to use on x86 (default: sse2)])],
  [case ${enableval} in
    sse|sse2|yes|no) ;;
    *) AC_MSG_ERROR([bad value ${enableval} for --enable-sse or --disable-sse]) ;;
  esac],
  [enable_sse=yes])

dnl We want XAPIAN_CXXFLAGS to be used for configure tests.
CXXFLAGS="$CXXFLAGS $XAPIAN_CXXFLAGS"

dnl We need to specify the argument types for builtin functions, or else
dnl AC_CHECK_DECLS fails to detect them when the compiler is clang.
AC_CHECK_DECLS([__builtin_add_overflow(int, int, int*),
		__builtin_sub_overflow(int, int, int*),
		__builtin_mul_overflow(int, int, int*)], [], [], [ ])
AC_CHECK_DECLS([__builtin_bswap16(uint16_t),
		__builtin_bswap32(uint32_t),
		__builtin_bswap64(uint64_t)], [], [],
	       [#include <stdint.h>])
AC_CHECK_DECLS([_byteswap_ushort, _byteswap_ulong, _byteswap_uint64], [], [],
	       [#include <stdlib.h>])
AC_CHECK_DECLS([__builtin_expect(long, long)], [], [], [ ])
AC_CHECK_DECLS([_addcarry_u32(unsigned char, unsigned, unsigned, unsigned*),
		_addcarry_u64(unsigned char, unsigned __int64, unsigned __int64, unsigned __int64*),
		_subborrow_u32(unsigned char, unsigned, unsigned, unsigned*),
		_subborrow_u64(unsigned char, unsigned __int64, unsigned __int64, unsigned __int64*)],
	       [], [], [#include <intrin.h>])

dnl Check for time functions.
AC_CHECK_FUNCS([clock_gettime sleep nanosleep gettimeofday ftime])

dnl Used by tests/harness/testsuite.cc
AC_CHECK_FUNCS([sigaction])
dnl These can be macros (and sigsetjmp is with glibc at least).
AC_CHECK_DECLS([sigsetjmp, siglongjmp], [], [], [#include <setjmp.h>])

dnl Used by tests/harness/cputimer.cc:
AC_CHECK_FUNCS([getrusage times sysconf])

dnl Used by tests/harness/unixcmd.cc
AC_CHECK_FUNCS([nftw])

dnl mingw (for instance) lacks ssize_t
AC_TYPE_SSIZE_T

AC_TYPE_PID_T

AC_TYPE_MODE_T

dnl Check for perl (needed to generate some sources and documentation).
AC_PATH_PROG([PERL], [perl], [])
AC_ARG_VAR([PERL], [Perl interpreter])
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
  AC_ARG_VAR([HELP2MAN], [help2man man page generator])
  test -z "$HELP2MAN" && AC_MSG_ERROR([help2man is required to build documentation])

  dnl Check for rst2html. (Needed to make HTML from reStructuredText format)
  dnl Also look for rst2html.py, which archlinux reportedly installs it as.
  AC_PATH_PROGS([RST2HTML], [rst2html rst2html.py], [])
  AC_ARG_VAR([RST2HTML], [reST to HTML convertor])
  test -z "$RST2HTML" && AC_MSG_ERROR([rst2html is required to build documentation (try package python-docutils)])
fi

dnl Checks for header files.
AC_CHECK_HEADERS([fcntl.h limits.h sys/select.h sysexits.h],
		 [], [], [ ])

dnl cxxabi.h was added in GCC 3.1, but clang lies and defines __GNUC__ yet
dnl doesn't seem to reliably provide this header, so probe for it.
AC_CHECK_HEADERS([cxxabi.h], [], [], [ ])

dnl If valgrind is installed and new enough, we use it for leak checking in the
dnl testsuite.  If VALGRIND is set to an empty value, then skip the check and
dnl don't use valgrind.  On macOS only use valgrind if VALGRIND is set to a
dnl no empty value, as valgrind on macOS gives a lot of false positives.
use_valgrind_if_non_empty=
case $host in
  *-*-darwin*)
    use_valgrind_if_non_empty=$VALGRIND ;;
  *)
    use_valgrind_if_non_empty=${VALGRIND-unset} ;;
esac

if test -n "$use_valgrind_if_non_empty" ; then
  AC_PATH_PROG([VALGRIND], [valgrind], [])
  AC_ARG_VAR([VALGRIND], [debugging tool (optionally used by test harness)])
  if test -n "$VALGRIND" ; then
    dnl Check that the installed valgrind version works, and supports the
    dnl options we use.  This means we won't try to use valgrind < 3.3.0
    dnl (released 7/12/2007) since before that --log-file didn't expand
    dnl %p (and appended the process id).
    dnl
    dnl No need to check for VALGRIND_COUNT_LEAKS now - that was added before
    dnl 2.0.0.
    AC_MSG_CHECKING([if valgrind supports --log-file with %p and --child-silent-after-fork=yes])
    vglog=config.vglog.%p.tmp
    vglogwild="config.vglog.*.tmp*"
    rm -f $vglogwild
    if $VALGRIND --log-file="$vglog" --child-silent-after-fork=yes -q true 2>&AS_MESSAGE_LOG_FD ; then
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

dnl If eatmydata is installed, we run the testsuite under it to speed it up.
dnl If EATMYDATA is set to an empty value, then skip this check and don't use
dnl eatmydata.
if test -n "${EATMYDATA-unset}" ; then
  AC_PATH_PROG([EATMYDATA], [eatmydata], [])
  AC_ARG_VAR([EATMYDATA], [wrapper to disable fsync (optionally used by test harness)])
fi

dnl Checks for library functions.
AC_FUNC_STRERROR_R

AC_CACHE_CHECK([for sys_errlist and sys_nerr], xo_cv_sys_errlist_and_sys_nerr, [
  AC_LINK_IFELSE([AC_LANG_PROGRAM(
dnl On Linux, sys_errlist and sys_nerr need <stdio.h>, though the man page
dnl says <errno.h>.  The man page seems to match other platforms such as
dnl NetBSD, so include both headers to ensure we get them.  With mingw and MSVC
dnl they're provided by <stdlib.h>.
[[#include <errno.h>
#include <stdio.h>
#include <stdlib.h>]],
[[  printf("%s", sys_errlist[sys_nerr - 1]);]])],
  [ xo_cv_sys_errlist_and_sys_nerr=yes ],
  [ xo_cv_sys_errlist_and_sys_nerr=no ])
])
if test "x$xo_cv_sys_errlist_and_sys_nerr" = "xyes" ; then
  AC_DEFINE([HAVE_SYS_ERRLIST_AND_SYS_NERR], [1],[Define if you have 'sys_errlist' and 'sys_nerr'])
fi

AC_CACHE_CHECK([for _sys_errlist and _sys_nerr], xo_cv__sys_errlist_and__sys_nerr, [
  AC_LINK_IFELSE([AC_LANG_PROGRAM(
[[#include <errno.h>
#include <stdio.h>
#include <stdlib.h>]],
[[  printf("%s", _sys_errlist[_sys_nerr - 1]);]])],
  [ xo_cv__sys_errlist_and__sys_nerr=yes ],
  [ xo_cv__sys_errlist_and__sys_nerr=no
  ])
])
if test "x$xo_cv__sys_errlist_and__sys_nerr" = "xyes" ; then
  AC_DEFINE([HAVE__SYS_ERRLIST_AND__SYS_NERR], [1],[Define if you have '_sys_errlist' and '_sys_nerr'])
fi

dnl GNU-specific replacement for sys_errlist and sys_nerr.
AC_CHECK_FUNCS([strerrordesc_np])

if true ; then

  case $host_os-$win32 in
    *-yes )
      dnl For mingw and msvc we have an alternative implementation which
      dnl doesn't need fork() or socketpair().
      ;;
    *djgpp* | *msdos* )
      dnl DJGPP has a dummy implementation of fork which always fails.  If
      dnl someone actually wanted remote backend support, then DJGPP has a
      dnl pthreads port, so using threads like we do on Windows would make more
      dnl sense.
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
      LIBS=$SAVE_LIBS
      ;;
  esac

fi

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

AC_MSG_CHECKING([for directory separator(s)])
case $host_os-$win32 in
  *-yes | cygwin* | os2* | *djgpp* | *msdos* )
    AC_MSG_RESULT([/ and \\])
    AC_DEFINE([DIR_SEPS], ["/\\"], [directory separator(s) as a character or string literal])
    AC_DEFINE([DIR_SEPS_LIST], [{ '/', '\\' }], [directory separator(s) as an initialiser list])
    ;;
  *)
    AC_MSG_RESULT([/])
    AC_DEFINE([DIR_SEPS], ['/'], [directory separator(s) as a character or string literal])
    AC_DEFINE([DIR_SEPS_LIST], [{ '/' }], [directory separator(s) as an initialiser list])
    ;;
esac

dnl ******************************
dnl * Set special compiler flags *
dnl ******************************

AC_ARG_ENABLE([werror],
  [AS_HELP_STRING([--enable-werror], [enable treating compiler warnings as errors [default=maintainer-mode]])],
  [case ${enableval} in
    yes|no) ;;
  *) AC_MSG_ERROR([bad value ${enableval} for --enable-werror]) ;;
  esac],
  [enable_werror=$USE_MAINTAINER_MODE])

dnl Put -Werror or equivalent in its own make variable so that it can easily
dnl be overridden by `make WERROR=` if needed during development (e.g. if
dnl you want to get a full list of compile warnings to fix rather than
dnl stopping at the first file with warnings).
WERROR=
AC_SUBST([WERROR])

dnl Set flags to control warnings (enable more, or disable annoying ones).
dash_d_visibility=
if test yes = "$GXX"; then
  dnl Intel's C++ compiler and clang both lie and define __GNUC__, so check which
  dnl we actually have, as neither is really 100% compatible.
  case `echo __INTEL_COMPILER __clang__|$CXX -E - 2>&AS_MESSAGE_LOG_FD|grep -v '^#'` in
    *__INTEL_COMPILER*__clang__*)
      dnl GCC (since neither substituted):

      dnl The exact format of g++ --version seems to change with almost every
      dnl minor release so use the preprocessor macros which should be more
      dnl robust.  Pass -xc++ to avoid warnings about any command-line options
      dnl included in $CXX which aren't valid for C (such as -std=c++17).
      AC_MSG_CHECKING([for version of $CXX])
      gxx_version=`echo __GNUC__.__GNUC_MINOR__.__GNUC_PATCHLEVEL__|$CXX -xc++ -E -|sed '/^#/d;s/ //g'`
      AC_MSG_RESULT([GCC $gxx_version])

      dnl -Wundef was supported by g++ 3.0.
      dnl
      dnl -Wlogical-op and -Wmissing-declarations (for C++) were added in
      dnl GCC 4.3.
      dnl
      dnl -Wdouble-promotion was added in GCC 4.6.
      dnl
      dnl -Wduplicated-cond was added in GCC 6.
      dnl
      dnl -Wduplicated-branches was added in GCC 7.
      dnl
      dnl All the other options were supported by g++ 2.95.
      AM_CXXFLAGS="$AM_CXXFLAGS -Wall -W -Wredundant-decls -Wpointer-arith -Wcast-qual -Wcast-align -Wformat-security -fno-gnu-keywords -Wundef -Woverloaded-virtual -Wstrict-null-sentinel -Wshadow -Wstrict-overflow=1 -Wlogical-op -Wmissing-declarations -Wdouble-promotion -Wduplicated-cond -Wduplicated-branches"

      case $gxx_version in
      [[0-6]].*)
	AC_MSG_ERROR([Xapian requires GCC 7.0 or later])
	;;
      esac

      dnl FIXME:
      dnl -Wold-style-cast is interesting, but triggers for macros from
      dnl system headers (e.g. FD_SET).  GCC 4.8 improved this by making
      dnl -ftrack-macro-expansion=2 the default, but we still see such
      dnl warnings on some platforms for some reason (e.g. Cygwin with GCC
      dnl 5.4.0 warns about FD_SET, and on Debian with GCC 4.9.2 we see
      dnl warnings from zlib.h).

      WERROR=-Werror
      ;;
    *__clang__*)
      dnl Intel's compiler (since __clang__ not substituted):
      dnl
      dnl -w1 stops the avalanche of uninteresting "remark" messages.
      dnl -wd... disables warnings which don't have good code workarounds.
      AM_CXXFLAGS="$AM_CXXFLAGS -Wall -w1 -wd177,1572"

      WERROR=-Werror
      ;;
    *__INTEL_COMPILER*)
      dnl clang (since __INTEL_COMPILER not substituted):

      dnl These options all work at least as far back as clang++ 3.0:
      AM_CXXFLAGS="$AM_CXXFLAGS -Wall -W -Wredundant-decls -Wpointer-arith -Wcast-qual -Wcast-align -Wformat-security -fno-gnu-keywords -Wundef -Woverloaded-virtual -Wshadow -Wstrict-overflow=1 -Wmissing-declarations -Winit-self"

      WERROR=-Werror
      ;;
  esac

  if test no != "$enable_visibility"; then
    dnl GCC doesn't support symbol visibility on all platforms (e.g. it isn't
    dnl supported on mingw and visibility attributes result in warnings).
    AC_MSG_CHECKING([if $CXX -fvisibility=hidden -fvisibility-inlines-hidden works])
    if echo 'int bar() __attribute__((visibility("default"))); int baz() __attribute__((visibility("internal"))); int bar() { return 6; } int baz() { return 7; } int foo() {return 42;}'|$CXX -Werror -fvisibility=hidden -fvisibility-inlines-hidden -c -oconftest.o -xc++ - >&AS_MESSAGE_LOG_FD 2>&AS_MESSAGE_LOG_FD ; then
      AC_MSG_RESULT([yes])
      AM_CXXFLAGS="$AM_CXXFLAGS -fvisibility=hidden -fvisibility-inlines-hidden"
      dash_d_visibility=-DXAPIAN_ENABLE_VISIBILITY
    else
      AC_MSG_RESULT([no])
    fi
  fi
elif test yes = "$msvc"; then
  dnl -W1 is command-line default, -W3 IDE default, -W4 highest numbered level.
  dnl AM_CXXFLAGS="$AM_CXXFLAGS -W4"

  WERROR=-WX
fi

if test x$enable_werror != xyes; then
  WERROR=
fi

XAPIAN_TEST_LDFLAGS([-Wl,-Bsymbolic-functions], [XAPIANLETOR_LDFLAGS])

AC_MSG_CHECKING([whether to use SSE instructions on x86])
case $host_cpu in
  i*86)
    if test "$enable_sse" = no ; then
      AC_MSG_RESULT([no])
      if test yes = "$msvc"; then
	dnl MSVC defaults to SSE2.
	AM_CXXFLAGS="$AM_CXXFLAGS -arch:IA32"
      fi
    else
      dnl Default to sse2.
      test "$enable_sse" != yes || enable_sse=sse2
      if test yes = "$GXX"; then
	AC_MSG_RESULT([yes (configure with --disable-sse to disable)])
	dnl We can unconditionally use -mtune=generic as it was added in GCC
	dnl 4.2, and supported at least as far back as clang 3.0.
	AM_CXXFLAGS="$AM_CXXFLAGS -mfpmath=sse -m$enable_sse -mtune=generic"
      elif test yes = "$msvc"; then
	AC_MSG_RESULT([yes (configure with --disable-sse to disable)])
	if test sse = "$enable_sse"; then
	  dnl MSVC defaults to SSE2.
	  AM_CXXFLAGS="$AM_CXXFLAGS -arch:SSE"
	fi
      else
	AC_MSG_RESULT([don't know how to for compiler $CXX])
      fi
    fi
    ;;
  *)
    AC_MSG_RESULT([non-x86 arch ($host_cpu)]) ;;
esac

if test "$GXX" = yes; then
  dnl Default to enabling _FORTIFY_SOURCE at level 2.  It shouldn't cause a
  dnl problem to define it where it's not supported, but we need to check for
  dnl an issue on mingw-w64 where some versions required linking with -lssp
  dnl (this was fixed in v11.0.0 released 2023-04-28).  We detect this with a
  dnl configure-time link test and don't auto-enable _FORTIFY_SOURCE if using
  dnl an affected version.
  AC_MSG_CHECKING([if _D_FORTIFY_SOURCE=2 links without extra libraries])
  save_CPPFLAGS=$CPPFLAGS
  CPPFLAGS="$CPPFLAGS -D_FORTIFY_SOURCE=2"
  AC_LINK_IFELSE([AC_LANG_PROGRAM([[
    #include <string.h>
    static char a[20], b[10];]], [[
    strcpy(a, b);
    ]])],
    [AC_MSG_RESULT([yes])
    AC_DEFINE([FORTIFY_SOURCE_OK], [1], [Define to 1 if '-D_FORTIFY_SOURCE=2' link test passes])],
    [AC_MSG_RESULT([no])])
  CPPFLAGS=$save_CPPFLAGS
fi

AH_BOTTOM(
[/* Disable stupid MSVC warnings. */
#ifdef _MSC_VER
/* Passing an empty parameter to a single parameter macro. */
# pragma warning(disable:4003)
/* Assignment in conditional expression; other compilers suppress with extra parentheses but not MSVC. */
# pragma warning(disable:4706)
/* A "performance" warning for converting int to bool. */
# pragma warning(disable:4800)

/* POSIX get to deprecate POSIX things, not Microsoft. */
# define _CRT_NONSTDC_NO_WARNINGS
# define _CRT_SECURE_NO_WARNINGS

#endif

/* MSVC defines _WIN32 but not __WIN32__. */
#if !defined __WIN32__ && defined _WIN32
# define __WIN32__
#endif

/* MSVC defines _WIN64 but not __WIN64__. */
#if !defined __WIN64__ && defined _WIN64
# define __WIN64__
#endif

/* Default to enabling _FORTIFY_SOURCE at level 2 for GCC-like compilers.  It
 * shouldn't cause a problem to define it where it's not supported.
 *
 * The FORTIFY_SOURCE_OK check avoids auto-enabling on some mingw-w64 versions
 * which required linking with -lssp too.  This was addressed in v11.0.0
 * (released 2023-04-28).
 *
 * Check if _FORTIFY_SOURCE is already defined to allow the user to override
 * our choice with "./configure CPPFLAGS=-D_FORTIFY_SOURCE=0" or "...=1" or
 * "...=3".
 */
#if defined FORTIFY_SOURCE_OK && !defined _FORTIFY_SOURCE
# define _FORTIFY_SOURCE 2
#endif

/* For compilers which support it (such as GCC, clang, Intel's C++ compiler)
 * we can use __builtin_expect to give the compiler hints about branch
 * prediction.  See HACKING for how to use these.
 */
#if HAVE_DECL___BUILTIN_EXPECT
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

dnl Required for auto regeneration to work in a combined maintainer-mode tree.
: ${AUTOM4TE=autom4te}
AC_SUBST([AUTOM4TE])

dnl We want to be able to use GNU make % pattern rules in maintainer targets
dnl but automake warns these aren't portable, so we substitute the % to avoid
dnl this warning.
PERCENT='%'
AC_SUBST([PERCENT])

dnl **************************
dnl * Build the output files *
dnl **************************

AC_CONFIG_FILES([
 Makefile
 tests/Makefile
 docs/Makefile
 cmake/xapianletor-config.cmake
 cmake/xapianletor-config-version.cmake
 ])
AC_CONFIG_FILES([makemanpage], [chmod +x makemanpage])
AC_CONFIG_FILES([tests/runtest], [chmod +x tests/runtest])
AC_CONFIG_FILES([xapianletor-config], [chmod +x xapianletor-config])
AC_OUTPUT
