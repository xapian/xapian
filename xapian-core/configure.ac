dnl Process this file with autoconf to produce a configure script.

dnl NB When updating the version for a release, update LIBRARY_VERSION_INFO
dnl below too.
AC_INIT([xapian-core], [1.4.25], [https://xapian.org/bugs])
dnl See HACKING document for details of the reasons for required versions.
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
dnl LIBRARY_VERSION_INFO for libxapian:
dnl 29:1:7 1.2.23 No API changes (except new inline methods)
dnl 30:0:0 1.4.0 Extensive API changes
dnl 31:0:1 1.4.1 New Weight subclasses, etc
dnl 32:0:2 1.4.2 Database::get_document() with flags
dnl 33:0:3 1.4.3 Database::locked() method
dnl 33:1:3 1.4.4 No API changes
dnl 34:0:4 1.4.5 Database::get_total_length() method
dnl 35:0:5 1.4.6 C++11 move support added
dnl 35:1:5 1.4.7 No API changes
dnl 36:0:6 1.4.8 STEM_SOME_FULL_POS added
dnl 36:1:6 1.4.9 No API changes
dnl 37:0:7 1.4.10 Added DatabaseClosedError, etc
dnl 38:0:8 1.4.11 Added SNIPPET_CJK_NGRAM
dnl 39:0:9 1.4.12 Database::size() method
dnl 39:1:9 1.4.13 No API changes
dnl 40:0:10 1.4.14 Xapian::Stem changes
dnl 40:1:10 1.4.15 No API changes
dnl 40:2:10 1.4.16 No API changes
dnl 40:3:10 1.4.17 No API changes (except a new inlined method)
dnl 41:0:11 1.4.18 QueryParser::FLAG_ACCUMULATE
dnl 42:0:12 1.4.19 QueryParser::FLAG_NO_POSITIONS
dnl 42:1:12 1.4.20 No API changes
dnl 42:2:12 1.4.21 No API changes
dnl 42:3:12 1.4.22 No API changes
dnl 42:4:12 1.4.23 No API changes
dnl 42:5:12 1.4.24 No API changes
dnl 42:6:12 1.4.25 No API changes
LIBRARY_VERSION_INFO=42:6:12
AC_SUBST([LIBRARY_VERSION_INFO])

LIBRARY_VERSION_SUFFIX=
AC_SUBST([LIBRARY_VERSION_SUFFIX])

dnl Disabled for stable release series.
test x"$program_suffix" != xNONE || program_suffix=

dnl Where xapian.h, etc go.  In development release append "/xapian-1.3".
incdir=$includedir
AC_SUBST([incdir])

dnl Check the build directory doesn't contain a space, so we die early with
dnl a helpful error.
case `pwd` in
*' '*)
   AC_MSG_ERROR([You can't build in a directory whose path contains a space])
   ;;
esac

dnl Check the source directory doesn't contain a space, so we die early with
dnl a helpful error.  FIXME: Unfortunately, configure seems to choke before
dnl it gets to us so this code doesn't get a chance to fire.
case $0 in
*' '*)
   dnl Note: for in-tree builds, the build directory test above will fire
   dnl before this can.
   AC_MSG_ERROR([You can't build with sources in a directory whose path contains a space])
   ;;
esac

dnl Check the prefix to install in doesn't contain a space, so we die early with
dnl a helpful error.
case $prefix in
*' '*)
   AC_MSG_ERROR([You can't install in a directory whose path contains a space])
   ;;
esac

dnl Note if the user specified a particular C++ compiler so we can give a more
dnl appropriate error message if we can't link a simple C++ program.
original_CXX=
if test -n "$CXX" ; then
  original_CXX="CXX=$CXX"
elif test -n "$CCC" ; then
  original_CXX="CCC=$CCC"
fi

dnl See HACKING document for details of the reasons for required versions.
AM_INIT_AUTOMAKE([1.11 -Wportability tar-ustar no-dist-gzip dist-xz std-options])
AC_CONFIG_SRCDIR([matcher/multiandpostlist.cc])

AC_CONFIG_HEADERS([config.h])

AC_CONFIG_MACRO_DIR([m4])

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

dnl -no-undefined causes problems on Solaris with Sun CC in C++11 mode, so only
dnl pass -no-undefined on platforms where it is required in order to link a
dnl shared library at all (Windows is the main one).
NO_UNDEFINED=
if test unsupported = "$allow_undefined_flag" ; then
  NO_UNDEFINED=-no-undefined
fi
AC_SUBST(NO_UNDEFINED)

AM_CXXFLAGS=

dnl A standard "gotcha" for mingw and cygwin users is to not set up their
dnl PATH correctly, so that MSDOS FIND.EXE is found before Unix find.  Help
dnl them out by checking for this condition, rather than letting libtool
dnl fail in obscure ways.  NB check the *BUILD* OS, not the host one!
case $build_os in
  cygwin* | mingw* | msys* | pw32*)
    find /dirunlikelytoexist >/dev/null 2>&1
    if test $? = 2 ; then
	dnl Unix find will return 1 if the directory didn't exist, or 0 if
	dnl it did.
	AC_MSG_ERROR([
*** You appear to have an MSDOS-like FIND.EXE in your PATH ahead of any
*** UNIX-like find.  This misconfiguration will confuse libtool - you'll need
*** to make sure you have a UNIX-like find installed and fix your PATH, then
*** rerun configure.  For more information, see:
***
***   https://www.cygwin.com/faq/faq.html#faq.using.find
])
    fi
  ;;
esac

dnl Add parameters for aclocal
dnl (This must come after AM_INIT_AUTOMAKE, since it modifies ACLOCAL)
ACLOCAL="$ACLOCAL $ACLOCAL_FLAGS"

dnl disable "maintainer only" rules by default
AM_MAINTAINER_MODE

dnl Checks for programs.
AC_PROG_CXX

AC_CANONICAL_HOST

# Checked: dragonfly6.4 freebsd8.0 openbsd4.6 solaris2.9 solaris2.10
case $host_os in
  linux* | k*bsd*-gnu | dragonfly* | freebsd* | openbsd* | solaris*)
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

dnl For reasons which are beyond me, if autoconf can't find a C++ compiler
dnl it will set CXX to g++ (which obviously won't work) rather than actually
dnl telling the user that it couldn't find a C++ compiler and telling them
dnl to either install one or set CXX if there's one configure failed to find.
dnl It's probably worthwhile checking that the C++ compiler actually works
dnl anyway!
if test -n "$CXX" ; then
  AC_LANG([C++])
  AC_MSG_CHECKING([whether $CXX is a working C++ compiler])
  AC_CACHE_VAL([xo_cv_cxx_works],
    [
    AC_RUN_IFELSE([AC_LANG_PROGRAM([[]], [[]])],
		  [xo_cv_cxx_works=yes],
		  [xo_cv_cxx_works=no],
		  [AC_LINK_IFELSE([AC_LANG_PROGRAM([[]], [[]])],
				  [xo_cv_cxx_works=yes],
				  [xo_cv_cxx_works=no])])
    ])
  AC_MSG_RESULT([$xo_cv_cxx_works])
else
  xo_cv_cxx_works=no
fi
if test no = "$xo_cv_cxx_works" ; then
  case $original_CXX in
  CCC=*)
    dnl CCC is handled in a slightly odd way - if its value isn't an
    dnl executable (taking PATH into account) then it is ignored!
    test "$original_CXX" = "CCC=$CXX" || original_CXX=
    ;;
  esac
  if test -z "$original_CXX" ; then
  AC_MSG_ERROR([
*** You need a working C++ compiler to compile Xapian, but configure failed to
*** find one.  If you have a working C++ compiler, you can tell configure where
*** to find it by invoking it like so:
***
***   ./configure CXX=/opt/bin/c++
])
  else
  AC_MSG_ERROR([
*** You need a working C++ compiler to compile Xapian, but the compiler you
*** specified (with '$original_CXX') doesn't appear to be able to successfully
*** compile and link a simple program.
])
  fi
fi

dnl Probe for any options needed to enable C++11 support.
AX_CXX_COMPILE_STDCXX([11])

dnl We don't use a C compiler to compile Xapian's code, but on some platforms
dnl (e.g. mingw) libtool uses $LTCC which defaults to $CC, and it's also use to
dnl build auxiliary tools like snowball and lemon.
AC_PROG_CC

dnl Check endianness.
AC_C_BIGENDIAN

dnl We want a non-cross-compiling C compiler for building lemon with.
if test -z "$CC_FOR_BUILD" ; then
  if test yes = "$cross_compiling"; then
    CC_FOR_BUILD=cc
  else
    CC_FOR_BUILD="$CC"
  fi
fi
AC_ARG_VAR([CC_FOR_BUILD], [C compiler command for native compilation (needed to compile build tools during cross-builds)])

dnl Run tests using the C++ compiler.
AC_LANG([C++])

dnl Enable large file support if possible.
AC_SYS_LARGEFILE
dnl With xlC on AIX, -D_LARGE_FILES changes the ABI of std::string, so it
dnl also needs to be used when compiling user code.
abi_affecting_cxxflags=
if $GREP '^#define _LARGE_FILES 1$' confdefs.h > /dev/null 2>&1 ; then
  abi_affecting_cxxflags=-D_LARGE_FILES
fi
AC_SUBST([abi_affecting_cxxflags])

AC_PREPROC_IFELSE([AC_LANG_SOURCE([[
#if defined __WIN32__ || defined _WIN32
#error WIN32
#endif
]])], [win32=no], [win32=yes])

AC_COMPILE_IFELSE([AC_LANG_PROGRAM([], [[
#ifdef _MSC_VER
#error MSVC
#endif
]])], [msvc=no], [msvc=yes])

XAPIAN_LDFLAGS=
XAPIAN_LIBS=
AC_SUBST([XAPIAN_LDFLAGS])
AC_SUBST([XAPIAN_LIBS])

xo_fn_unknown_option_check() {
  dnl Some compilers don't error out on unknown options which unhelpfully
  dnl means we would think the option is supported, use it, and end up
  dnl with a warning or note diagnostic every time we use the option.
  dnl
  dnl Check compiler stderr for known strings:
  dnl * MSVC: Command line warning D9002 : ignoring unknown option '...'
  dnl * icc strings cribbed from automake's depend.m4
  $GREP 'ignoring unknown option' conftest.err >/dev/null 2>&1 ||
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

ldflags=
if test yesyes = "$GXX$enable_shared" ; then
  case $host_os in
    cygwin* | mingw* | msys*)
      XAPIAN_TEST_LDFLAGS(
	[-Wl,--enable-runtime-pseudo-reloc],
	[ldflags],
	[],
	[
	dnl Can't use AC_DISABLE_SHARED after AC_PROG_LIBTOOL, but
	dnl this test needs to be after AC_PROG_LIBTOOL, so we can't
	dnl just disable the shared build automatically...
	AC_MSG_ERROR([ld version too old to support a shared build - configure with --disable-shared, or install binutils 2.13.90-20030111-1 or later])
	])
    ;;
  esac
fi
dnl Only works for ldflags which can be specified anywhere on the link line.
AC_SUBST([ldflags])

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

dnl The maths functions we use include: exp log ceil fabs sqrt
AC_MSG_CHECKING([if -lm is required for maths functions])
dnl Don't use constant arguments as the compiler might simply evaluate the
dnl whole expression at compile time, and it might inline certain functions
dnl so test several functions.  Also write results using printf() so that
dnl the compiler can't optimise away the computations.
AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <cmath>
#include <cstdio>
#include <ctime>
using namespace std;]], [[
  double a = log(ceil(time(NULL)/7.0));
  printf("%f %f %f\n", a, sqrt(a), exp(fabs(a - 12345.6)));
  ]])], [AC_MSG_RESULT([no])], [
    LIBS="-lm $LIBS"
    AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <cmath>
#include <cstdio>
#include <ctime>
using namespace std;]], [[
  double a = log(ceil(time(NULL)/7.0));
  printf("%f %f %f\n", a, sqrt(a), exp(fabs(a - 12345.6)));]])],
	[AC_MSG_RESULT([yes])],
	[AC_MSG_ERROR([Failed to link a C++ program using <cmath>])
    ])
])

dnl C++11 should have log2(double), but keep this check for now to allow for
dnl compilers with incomplete library support for C++11.
AC_CHECK_DECLS([log2(double)], [], [],
[#include <cmath>
using namespace std;])

dnl exp10() is a GNU libc extension.
AC_CHECK_DECLS([exp10(double)], [], [],
[#include <cmath>
using namespace std;])

dnl Darwin provides __exp10().
AC_CHECK_DECLS([__exp10(double)], [], [],
[#include <cmath>
using namespace std;])

dnl GCC provides __builtin_exp10() (but notably clang doesn't currently).
dnl On mingw, __builtin_exp10() causes GCC to generate a call to exp10() which
dnl then fails to link, so perform a link check here to catch that.
AC_CACHE_CHECK([for __builtin_exp10], xo_cv_link___builtin_exp10, [
  AC_LINK_IFELSE([AC_LANG_PROGRAM([[
#include <cstdio>
#include <ctime>
using namespace std;]],
    [[printf("%f", __builtin_exp10(double(time(NULL)*1e-8)));]])],
    [xo_cv_link___builtin_exp10=yes],
    [xo_cv_link___builtin_exp10=no])
])
if test $xo_cv_link___builtin_exp10 = yes ; then
  AC_DEFINE([HAVE___BUILTIN_EXP10], [1], [Define to 1 if you have the '__builtin_exp10' function.])
fi

dnl See if <typeinfo> can be used in the testsuite - at least for GCC and xlC,
dnl compilation of the test code below fails if RTTI isn't being generated
dnl (g++ -fno-rtti, or by default with xlC).
AC_MSG_CHECKING([if RTTI is supported])
save_CXXFLAGS=$CXXFLAGS
dnl xlC issues a warning for typeid() being used without RTTI being enabled,
dnl so for this test we pass the xlC option to make that warning into an error.
CXXFLAGS="$CXXFLAGS -qhaltonmsg=1540-2412"
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[]], [[]])],
      [],
      [CXXFLAGS=$save_CXXFLAGS])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <exception>
#include <typeinfo>]],
	[[
	int f();
	try {
	    return f();
	} catch (std::exception & e) {
	    return typeid(e).name()[0];
	}]])],
    [AC_MSG_RESULT([yes])
    AC_DEFINE([USE_RTTI], [1], [Define if the testsuite can use RTTI])],
    [AC_MSG_RESULT([no])])
CXXFLAGS=$save_CXXFLAGS

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
AC_CHECK_DECLS([__builtin_clz(unsigned),
		__builtin_clzl(unsigned long),
		__builtin_clzll(unsigned long long)], [], [], [ ])
AC_CHECK_DECLS([__builtin_ctz(unsigned),
		__builtin_ctzl(unsigned long),
		__builtin_ctzll(unsigned long long)], [], [], [ ])
AC_CHECK_DECLS([__builtin_expect(long, long)], [], [], [ ])
AC_CHECK_DECLS([__builtin_popcount(unsigned),
		__builtin_popcountl(unsigned long),
		__builtin_popcountll(unsigned long long)], [], [], [ ])
AC_CHECK_DECLS([__popcnt, __popcnt64], [], [], [#include <intrin.h>])
AC_CHECK_DECLS([_addcarry_u32(unsigned char, unsigned, unsigned, unsigned*),
		_addcarry_u64(unsigned char, unsigned __int64, unsigned __int64, unsigned __int64*),
		_subborrow_u32(unsigned char, unsigned, unsigned, unsigned*),
		_subborrow_u64(unsigned char, unsigned __int64, unsigned __int64, unsigned __int64*)],
	       [], [], [#include <intrin.h>])

dnl Check for poll().
AC_CHECK_FUNCS([poll])

dnl Check for time functions.
AC_CHECK_FUNCS([clock_gettime sleep nanosleep gettimeofday ftime])

case $host_os in
  *mingw*)
    AC_DEFINE([MINGW_HAS_SECURE_API], [1], [Define on mingw to get _s suffixed "secure" functions declared in headers])
    ;;
esac

dnl Used by tests/harness/testsuite.cc
AC_CHECK_FUNCS([sigaction])
dnl These can be macros (and sigsetjmp is with glibc at least).
AC_CHECK_DECLS([sigsetjmp, siglongjmp], [], [], [#include <setjmp.h>])

dnl Used by tests/harness/cputimer.cc:
AC_CHECK_FUNCS([getrusage times sysconf])

dnl Used by tests/harness/unixcmd.cc
AC_CHECK_FUNCS([nftw])

dnl POSIX requires setenv().  The final Unix-like platform without it seems
dnl to have been Solaris 9, which is now out of support.
dnl
dnl Microsoft don't provide setenv() and have marked putenv() as deprecated, so
dnl we use _putenv_s() to avoid deprecation warnings with MSVC.  It's also a
dnl more similar interface to setenv() so easier to implement a setenv() wrapper
dnl around.  We probe for both the function and a declaration since mingw
dnl doesn't currently (v3.20) declare it but it is present in the C runtime DLL
dnl so we can provide our own declaration to access it.
AC_CHECK_FUNCS([setenv _putenv_s])
AC_CHECK_DECLS([_putenv_s(const char*, const char*)], [], [],
[#include <stdlib.h>])

dnl See if we have closefrom(), or some functions that are useful to implement
dnl closefrom() on platforms which don't provide it.
AC_CHECK_FUNCS([closefrom getdirentries getrlimit])

if test $ac_cv_func_ftime = yes ; then
  dnl See if ftime returns void (as it does on mingw)
  AC_MSG_CHECKING([return type of ftime])
  AC_COMPILE_IFELSE(
    [AC_LANG_PROGRAM([[#include <sys/timeb.h>]],
		     [[struct timeb tp; int i = ftime(&tp);]])],
    [AC_MSG_RESULT([int])],
    [AC_MSG_RESULT([void])
    AC_DEFINE([FTIME_RETURNS_VOID], [1], [Define if ftime returns void])])
fi

dnl Check how to find the hostname: uname() in sys/utsname.h, or gethostname()
AC_CHECK_HEADERS([sys/utsname.h], [], [], [ ])
AC_CHECK_FUNCS([gethostname])

dnl mingw (for instance) lacks ssize_t
AC_TYPE_SSIZE_T

AC_TYPE_PID_T

AC_TYPE_MODE_T

AC_CHECK_SIZEOF([short])
AC_CHECK_SIZEOF([int])
AC_CHECK_SIZEOF([long])
AC_CHECK_SIZEOF([long long])

AC_CHECK_HEADERS([sys/types.h])

AC_MSG_CHECKING([for 32 bit integer type])
case 4 in
    "$ac_cv_sizeof_int")   INT32_T=int ;;
    "$ac_cv_sizeof_long")  INT32_T=long ;;
    "$ac_cv_sizeof_short") INT32_T=short ;;
    *)
      AC_MSG_RESULT([none found])
      AC_MSG_ERROR([No 32 bit integer type found])
esac
AC_MSG_RESULT([$INT32_T])

AC_MSG_CHECKING([for 64 bit integer type])
case 8 in
    "$ac_cv_sizeof_int")       INT64_T=int ;;
    "$ac_cv_sizeof_long")      INT64_T=long ;;
    "$ac_cv_sizeof_long_long") INT64_T='long long' ;;
    *)
      AC_MSG_RESULT([none found])
      AC_MSG_ERROR([No 64 bit integer type found])
esac
AC_MSG_RESULT([$INT64_T])

dnl Check for perl (needed to generate some sources and documentation).
AC_PATH_PROG([PERL], [perl], [])
AC_ARG_VAR([PERL], [Perl interpreter])
if test x$USE_MAINTAINER_MODE = xyes; then
  test -z "$PERL" && AC_MSG_ERROR([perl is required in maintainer mode])
fi

AC_ARG_ENABLE([64bit_docid],
  [AS_HELP_STRING([--enable-64bit-docid], [enable 64bit docid])],
  [case ${enableval} in
    yes|no) ;;
  *) AC_MSG_ERROR([bad value ${enableval} for --enable-64bit-docid]) ;;
  esac],
  [enable_64bit_docid=no])

AC_ARG_ENABLE([64bit_termcount],
  [AS_HELP_STRING([--enable-64bit-termcount], [enable 64bit termcount])],
  [case ${enableval} in
    yes|no) ;;
  *) AC_MSG_ERROR([bad value ${enableval} for --enable-64bit-termcount]) ;;
  esac],
  [enable_64bit_termcount=no])

AC_ARG_ENABLE([64bit_termpos],
  [AS_HELP_STRING([--enable-64bit-termpos], [enable 64bit termpos])],
  [case ${enableval} in
    yes|no) ;;
  *) AC_MSG_ERROR([bad value ${enableval} for --enable-64bit-termpos]) ;;
  esac],
  [enable_64bit_termpos=no])

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
  dnl Checks for dot.  (Diagrams in the documentation)
  AC_PATH_PROG([DOT], [dot])
  AC_ARG_VAR([DOT], [dot from graphviz])
  test -z "$DOT" && AC_MSG_ERROR([dot (part of the graphviz package) is required to build documentation])
  DOXYGEN_DOT_PATH=`echo "$DOT" | sed 's!/dot$!!'`
  AC_SUBST([DOXYGEN_DOT_PATH])

  dnl Check for doxygen. (Needed to make some more of the documentation)
  AC_PATH_PROG([DOXYGEN], [doxygen], [])
  AC_ARG_VAR([DOXYGEN], [Doxygen documentation system])
  test -z "$DOXYGEN" && AC_MSG_ERROR([doxygen is required to build documentation])

  dnl Check for help2man. (Needed to make man pages from "--help" output).
  AC_PATH_PROG([HELP2MAN], [help2man], [])
  AC_ARG_VAR([HELP2MAN], [help2man man page generator])
  test -z "$HELP2MAN" && AC_MSG_ERROR([help2man is required to build documentation])

  dnl Check for rst2html. (Needed to make HTML from reStructuredText format)
  dnl Also look for rst2html.py, which archlinux reportedly installs it as.
  AC_PATH_PROGS([RST2HTML], [rst2html rst2html.py], [])
  AC_ARG_VAR([RST2HTML], [reST to HTML convertor])
  test -z "$RST2HTML" && AC_MSG_ERROR([rst2html is required to build documentation (try package python-docutils)])

  dnl Check for pngcrush, which we optionally use to reduce the size of the
  dnl PNG files which doxygen generates.  We can get by without it, so don't
  dnl fail here if it's not found.
  AC_PATH_PROG([PNGCRUSH], [pngcrush], [])
  AC_ARG_VAR([PNGCRUSH], [PNG optimiser])
fi

dnl Checks for header files.
AC_CHECK_HEADERS([fcntl.h limits.h poll.h sys/select.h sysexits.h],
		 [], [], [ ])
AC_CHECK_HEADERS([sys/resource.h],
		 [], [], [#include <sys/types.h>])

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

dnl mingw has _snprintf so check for that too.
AC_MSG_CHECKING([for  snprintf])
AC_CACHE_VAL([xo_cv_func_snprintf],
  [
  xo_cv_func_snprintf=no
  for func in snprintf _snprintf ; do
    AC_COMPILE_IFELSE([
      AC_LANG_PROGRAM(
	[[
#include <cstdio>
using namespace std;
	]],
	[[
	  char buffer[4] = "abc";
	  int res = $func(buffer, 4, "%s", "XYZ");
	  return res;
	]]
      )],
      [xo_cv_func_snprintf=$func;break],
      []
    )
  done
  ])
AC_MSG_RESULT([$xo_cv_func_snprintf])
if test "$xo_cv_func_snprintf" != no ; then
  AC_DEFINE_UNQUOTED([SNPRINTF], [$xo_cv_func_snprintf],
     [Define to the name of a function implementing snprintf but not caring about ISO C99 return value semantics (if one exists)])
fi

dnl ***************************
dnl * Select modules to build *
dnl ***************************

dnl Check which database backends should be built.

AC_DEFUN([_XAPIAN_BACKEND_ENABLE],
  [AC_ARG_ENABLE([backend_$1],
    [AS_HELP_STRING([--enable-backend-$1], [build the $1 database backend [default=$3]])],
    [case $enableval in
      yes|no) ;;
      *) AC_MSG_ERROR([Invalid option: '--enable-backend-$1=$enableval']) ;;
    esac], [enable_backend_$1=$2])
  ])

AC_DEFUN([XAPIAN_BACKEND_ENABLE],
  [_XAPIAN_BACKEND_ENABLE([$1],
    m4_default([$2], [yes]),
    m4_default([$3], [yes]))
  ])

dnl When adding a new backend, update INSTALL too.
XAPIAN_BACKEND_ENABLE([chert])
XAPIAN_BACKEND_ENABLE([glass])
XAPIAN_BACKEND_ENABLE([inmemory])
default_enable_backend_remote=yes
case $host_os in
  *djgpp* | *msdos* )
    dnl We don't currently support the remote backend on this platform so
    dnl disable it by default.
    default_enable_backend_remote=no
    ;;
esac
XAPIAN_BACKEND_ENABLE([remote], [$default_enable_backend_remote], [yes (except for MSDOS)])

dnl Save and empty LIBS while we probe for libraries to link libxapian against
dnl so we can just use AC_SEARCH_LIBS then copy LIBS to XAPIAN_LIBS and restore
dnl the value saved here.
save_LIBS=$LIBS
LIBS=

dnl See if we have fdatasync, and what libraries are needed for it.
dnl We need to actually check for a declaration as macOS has a dummy
dnl implementation in the library which is not prototyped in any header.
AC_CHECK_DECL([fdatasync(int)], [
  AC_SEARCH_LIBS([fdatasync], [rt], [])
  AC_CHECK_FUNCS([fdatasync])
  ],
  [ac_cv_func_fdatasync=no],
  [#include <unistd.h>]
)

dnl We use timer_create() if available to implement a search time limit.
AC_SEARCH_LIBS([timer_create], [rt],
	       [
    AC_MSG_CHECKING([for timer_create() usability])
    dnl Several platforms have timer_create() but it's non-functional.
    dnl
    dnl Some of these (at least those with a stub implementation which always
    dnl fails with ENOSYS) could be probed for, but we'd have to run code
    dnl which isn't possible when cross-compiling, so just maintain a list
    dnl of such platforms for now.
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM(
[[#if defined _AIX
#error timer_create always fails with EAGAIN on AIX 7.1
#elif defined __NetBSD__
#error SIGEV_THREAD not implemented in NetBSD 9.2 (see: man sigevent)
#elif defined __OpenBSD__
#error timer_create always fails with ENOSYS on OpenBSD 7.3 (and no prototype)
#endif]])],
	[AC_MSG_RESULT([yes])
	AC_DEFINE([HAVE_TIMER_CREATE], [1], [Define to 1 if you have the 'timer_create' function.])]
	,
	[AC_MSG_RESULT([no])
    ])
])

win32_need_lws2_32=0
case $enable_backend_chert$enable_backend_glass in
*yes*)
  dnl We use zlib for compressing tags in chert/glass.  We could
  dnl automatically disable support if zlib isn't found, but overall that
  dnl probably does more harm than good - it's most likely that someone just
  dnl forgot to install the -dev package for zlib.
  dnl
  dnl Similarly for uuid support.

  dnl Check for zlib.h.
  AC_CHECK_HEADERS([zlib.h], [], [
    AC_MSG_ERROR([zlib.h not found - required for chert and glass (you may need to install the zlib1g-dev or zlib-devel package)])
    ], [ ])

  dnl Check for zlibVersion - normally it's in -lz but the mingw build needs -lzlib or -lzdll.
  AC_SEARCH_LIBS([zlibVersion], [z zlib zdll], [], [
    AC_MSG_ERROR([zlibVersion() not found in -lz, -lzlib, or -lzdll - required for chert and glass (you may need to install the zlib1g-dev or zlib-devel package)])
    ])

  dnl Find a way to generate UUIDs.

  case $host_os-$win32 in
    *-yes | cygwin* | msys*) dnl Use built-in win32 API.
      AC_DEFINE([USE_WIN32_UUID_API], [1],
		[Define to 1 to use UuidCreate(), etc])
      if test $win32 = yes ; then
	dnl We need -lws2_32 for htonl(), etc, but not for Cygwin which
	dnl provides these functions in its libc.
	win32_need_lws2_32=1
      fi
      LIBS="$LIBS -lrpcrt4"
      ;;
    *)
      dnl Check for uuid/uuid.h (e2fsprogs/util-linux-ng) or uuid.h
      dnl (FreeBSD/NetBSD/OpenBSD/AIX).
      AC_CHECK_HEADERS([uuid/uuid.h], [
	dnl util-linux-ng's uuid/uuid.h found - check for uuid_generate in
	dnl -luuid.
	AC_SEARCH_LIBS([uuid_generate], [uuid], [], [
	  AC_MSG_ERROR([uuid_generate() not found in -luuid - required for chert and glass (you may need to install the uuid-dev, libuuid-devel or e2fsprogs-devel package)])
	  ])
      ], [
	dnl Try uuid.h as found on FreeBSD/NetBSD/OpenBSD/AIX, with associated
	dnl code in libc.
	AC_CHECK_HEADERS([uuid.h], [
	  dnl Check for uuid_create with no extra libraries required.
	  AC_CHECK_FUNC([uuid_create], [], [
	    AC_MSG_ERROR([uuid.h found, but uuid_create() not found.  You probably wants to install libuuid from util-linux-ng or e2fsprogs (you may need to install the uuid-dev, libuuid-devel or e2fsprogs-devel package)])
	  ])
	], [
	  dnl Especially useful for Android.
	  AC_MSG_CHECKING([if host platform supports /proc/sys/kernel/random/uuid])
	  case $host_os in
	    linux*)
	      AC_DEFINE([USE_PROC_FOR_UUID], [1],
			[Define to 1 to read UUID from '/proc/sys/kernel/random/uuid'])
	      AC_MSG_RESULT([yes])
	      ;;
	    *)
	      AC_MSG_RESULT([no])
	      AC_MSG_ERROR([Failed to find a way to generate UUIDs, required for chert and glass backends (you may need to install the uuid-dev, libuuid-devel or e2fsprogs-devel package)])
	      ;;
	  esac
	], [ ])
      ], [ ])
      ;;
  esac

  case $host_os in
    *djgpp* | *msdos* )
      dnl DJGPP has a dummy implementation of fork which always fails.
      dnl
      dnl For disk-based backends, use flock() for locking, which doesn't need
      dnl fork() or socketpair().
      AC_DEFINE([FLINTLOCK_USE_FLOCK], 1, [Define to use flock() for flint-compatible locking])
      ;;
  esac
  ;;
esac

if test "$enable_backend_remote" = yes ; then
  case $host_os-$win32 in
    *-yes )
      dnl For mingw and msvc we have an alternative implementation which
      dnl doesn't need fork() or socketpair().
      dnl
      dnl We need -lws2_32 for getaddrinfo(), etc.
      win32_need_lws2_32=1
      dnl Vista is needed for the AI_ADDRCONFIG flag to getaddrinfo().
      AC_DEFINE([WINVER], [0x600],
		[Version of Windows to assume (0x600 => Vista).])
      AC_DEFINE([_WIN32_WINNT], [WINVER],
		[Version of Windows to assume.])
      ;;
    *djgpp* | *msdos* )
      dnl DJGPP has a dummy implementation of fork which always fails.  If
      dnl someone actually wanted remote backend support, then DJGPP has a
      dnl pthreads port, so using threads like we do on Windows would make more
      dnl sense.
      AC_MSG_ERROR([The remote backend is not currently supported on this platform.])
      ;;
    *)
      dnl On Unix, we need fork and socketpair for the remotebackend.
      AC_CHECK_FUNCS([fork], [], [
	AC_MSG_ERROR([fork() required for the remote backend - if an extra library is needed, pass LIBS=-lfoo to configure.  Or --disable-backend-remote to disable it.])
      ])
      dnl Check if -lsocket is required for socketpair (Solaris needs it).
      dnl And on Haiku it's in -lnetwork.
      AC_SEARCH_LIBS([socketpair], [socket network], [], [
	AC_MSG_ERROR([socketpair() required for the remote backend - if an extra library is needed, pass LIBS=-lfoo to configure.  Or --disable-backend-remote to disable it.])
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
	AC_MSG_ERROR([getaddrinfo() required for the remote backend - if an extra library is needed, pass LIBS=-lfoo to configure.  Or --disable-backend-remote to disable it.])
      ])
      AC_SEARCH_LIBS([inet_ntop], [nsl socket], [], [
	AC_MSG_ERROR([inet_ntop() required for the remote backend - if an extra library is needed, pass LIBS=-lfoo to configure.  Or --disable-backend-remote to disable it.])
      ])
      ;;
  esac

  XAPIAN_TYPE_SOCKLEN_T
fi

if test "$win32_need_lws2_32" = 1 ; then
  LIBS="$LIBS -lws2_32"
fi
XAPIAN_LIBS=$LIBS
LIBS=$save_LIBS

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

dnl Turn off compilation of anything that we don't have the requirements for

dnl Set conditionals to specify what we compile

AM_CONDITIONAL([BUILD_BACKEND_CHERT], [test yes = "$enable_backend_chert"])
AM_CONDITIONAL([BUILD_BACKEND_GLASS], [test yes = "$enable_backend_glass"])
AM_CONDITIONAL([BUILD_BACKEND_INMEMORY], [test yes = "$enable_backend_inmemory"])
AM_CONDITIONAL([BUILD_BACKEND_REMOTE], [test yes = "$enable_backend_remote"])
AM_CONDITIONAL([BUILD_BACKEND_CHERT_OR_GLASS],
  [test nono != "$enable_backend_chert$enable_backend_glass"])

dnl Decide if we should use the zlib-vg.so LD_PRELOAD hack.
use_zlib_vg=no
if test -n "$VALGRIND" ; then
  case $host_os in
    linux*) use_zlib_vg=yes ;;
  esac
fi
AM_CONDITIONAL([USE_ZLIB_VG], [test yes = "$use_zlib_vg"])

AC_CHECK_FUNCS([fsync])
AC_CHECK_FUNCS([posix_fadvise])
if test "$win32" = no ; then
  dnl ftruncate() under Wine seems to be buggy and sometimes fails, though
  dnl a cut-down reproducer seems fine.  For now just avoid ftruncate()
  dnl entirely on this platform - we'll fallback to lseek() + write() of a
  dnl single zero byte, which is pretty efficient.
  AC_CHECK_FUNCS([ftruncate])
fi

dnl HP-UX has pread and pwrite, but they don't work!  Apparently this problem
dnl manifests when largefile support is enabled, and we definitely want that
dnl so don't use pread or pwrite on HP-UX.
case $host_os in
  hpux*)
    AC_MSG_CHECKING([for pread])
    AC_MSG_RESULT([present but broken on $host_os])
    AC_MSG_CHECKING([for pwrite])
    AC_MSG_RESULT([present but broken on $host_os])
    ;;
  *)
    AC_CHECK_FUNC([pread],
       [AC_DEFINE([HAVE_PREAD], [1],
	    [Define if pread is available on this system])
	AC_MSG_CHECKING([for any prototype needed for pread])
	AC_CACHE_VAL([xo_cv_pread_prototype],
	  [
	    for p in ' ' \
	      'extern "C" ssize_t pread(int, void *, size_t, off_t) throw ();' \
	      'extern "C" ssize_t pread(int, void *, size_t, off_t);' ; do
	      AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <sys/types.h>
#include <unistd.h>
$p
	      ]],[[
		char b[256];
		pread(1, b, 256, 20);
	      ]])],[
		xo_cv_pread_prototype="$p"
		break
	      ], [])
	    done
	    if test -z "$xo_cv_pread_prototype"; then
	      AC_MSG_RESULT([not found])
	      AC_MSG_ERROR([Failed to find working prototype for pread])
	    fi
	  ])
	  if test " " = "$xo_cv_pread_prototype" ; then
	    AC_MSG_RESULT([none required])
	  else
	    AC_MSG_RESULT([$xo_cv_pread_prototype])
	    AC_DEFINE_UNQUOTED([PREAD_PROTOTYPE], [$xo_cv_pread_prototype],
			       [explicit prototype needed for pread (if any)])
	  fi
	])
    AC_CHECK_FUNC([pwrite],
       [AC_DEFINE([HAVE_PWRITE], [1],
	    [Define if pwrite is available on this system])
	AC_MSG_CHECKING([for any prototype needed for pwrite])
	AC_CACHE_VAL([xo_cv_pwrite_prototype],
	  [
	    for p in ' ' \
	      'extern "C" ssize_t pwrite(int, const void *, size_t, off_t) throw ();' \
	      'extern "C" ssize_t pwrite(int, const void *, size_t, off_t);' ; do
	      AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <sys/types.h>
#include <unistd.h>
$p
	      ]],[[
		const char *p = "hello";
		pwrite(1, p, 5, 20);
	      ]])],[
		xo_cv_pwrite_prototype="$p"
		break
	      ], [])
	    done
	    if test -z "$xo_cv_pwrite_prototype"; then
	      AC_MSG_RESULT([not found])
	      AC_MSG_ERROR([Failed to find working prototype for pwrite])
	    fi
	  ])
	  if test " " = "$xo_cv_pwrite_prototype" ; then
	    AC_MSG_RESULT([none required])
	  else
	    AC_MSG_RESULT([$xo_cv_pwrite_prototype])
	    AC_DEFINE_UNQUOTED([PWRITE_PROTOTYPE], [$xo_cv_pwrite_prototype],
			       [explicit prototype needed for pwrite (if any)])
	  fi
	])
    ;;
esac

AC_MSG_CHECKING([for directory separator(s)])
case $host_os-$win32 in
  *-yes | cygwin* | msys* | os2* | *djgpp* | *msdos* )
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

AC_CHECK_FUNCS([link])

dnl *************************
dnl * Set debugging options *
dnl *************************

dnl Which assertion types to enable in the code.

AC_ARG_ENABLE([assertions],
  [AS_HELP_STRING([--enable-assertions], [enable debug assertions (no|partial|yes) [default=no]])],
  [case $enableval in
    yes|partial|no) ;;
    *)
      AC_MSG_ERROR([Invalid option: '--enable-assertions=$enableval']) ;;
  esac])

AC_ARG_ENABLE([log],
  [AS_HELP_STRING([--enable-log], [generate a log of methods called, etc (no|yes|profile) [default=no]])],
  [case $enableval in
    yes|no) ;;
    profile)
      AC_MSG_ERROR(['--enable-log=profile' is no longer supported - see https://trac.xapian.org/wiki/ProfilingXapian for alternatives.]) ;;
    *)
      AC_MSG_ERROR([Invalid option: '--enable-log=$enableval']) ;;
  esac])

dnl Set defines according to the --enable-assertions and --enable-log options
dnl given.

if test yes = "$enable_assertions" || test partial = "$enable_assertions" ; then
  AC_DEFINE([XAPIAN_ASSERTIONS],,
    [Define if you want assertions (causes some slow-down)])
fi

if test yes = "$enable_assertions"; then
  AC_DEFINE([XAPIAN_ASSERTIONS_PARANOID],,
    [Define if you want paranoid assertions (causes significant slow-down)])
fi

if test yes = "$enable_log"; then
  AC_DEFINE([XAPIAN_DEBUG_LOG],,
    [Define if you want a log of methods called and other debug messages])
fi

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
      dnl -Winit-self was added in GCC 3.4, but for GCC < 4.7 g++ always
      dnl warns for this case with -Wuninitialized (implied by -W).  We
      dnl don't intend to use this idiom, so any instances are bugs we'd
      dnl like to know about.
      dnl
      dnl All the other options were supported by g++ 2.95.
      AM_CXXFLAGS="$AM_CXXFLAGS -Wall -W -Wredundant-decls -Wpointer-arith -Wcast-qual -Wcast-align -Wformat-security -fno-gnu-keywords -Wundef -Woverloaded-virtual -Wstrict-null-sentinel -Wshadow -Wstrict-overflow=1 -Wlogical-op -Wmissing-declarations -Wdouble-promotion -Winit-self"

      case $gxx_version in
      [[0123]].*|4.[[0-6]].*)
	AC_MSG_ERROR([Xapian requires GCC 4.7 or later])
	;;
      4.*|5.*)
	;;
      6.*)
	AM_CXXFLAGS="$AM_CXXFLAGS -Wduplicated-cond"
	;;
      *)
	AM_CXXFLAGS="$AM_CXXFLAGS -Wduplicated-cond -Wduplicated-branches"
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
  WERROR=-WX
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
  */sunCC|*/CC)
    dnl Sun's C++ compiler.
    dnl
    dnl We use -errtags to report the tag for each warning emitted.
    dnl
    dnl We use -erroff to suppress some unhelpful warnings:
    dnl
    dnl oklambdaretmulti warns about a lambda with more than one return
    dnl statement even if they all return the same type.
    dnl
    dnl voidretw warns about functions failing to return a value even when the
    dnl function ends with a "throw" statement.  That's just unhelpful noise,
    dnl and adding a dummy "return" after "throw" seems a worse solution than
    dnl simply disabling this warning - any genuinely missing return statements
    dnl will get reported by compilers with superior warning machinery.
    XAPIAN_TEST_CXXFLAGS([-errtags -erroff=oklambdaretmulti,voidretw], [AM_CXXFLAGS])
    ;;
  esac
fi

if test x$enable_werror != xyes; then
  WERROR=
fi

XAPIAN_TEST_LDFLAGS([-Wl,-Bsymbolic-functions], [XAPIAN_LDFLAGS])
dnl Roughly equivalent Sun C++ option.
XAPIAN_TEST_LDFLAGS([-xldscope=symbolic], [XAPIAN_LDFLAGS])

dnl New in GCC 5.  Sort of like -Bsymbolic-functions, but for the compiler
dnl rather than the linker.
XAPIAN_TEST_LDFLAGS([-fno-semantic-interposition], [XAPIAN_LDFLAGS])

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
	AC_COMPILE_IFELSE([AC_LANG_PROGRAM(
[[#ifndef __SUNPRO_CC
#error Not Sun compiler
#endif]])],
	  [AC_MSG_RESULT([yes (configure with --disable-sse to disable)])
	  AM_CXXFLAGS="$AM_CXXFLAGS -xarch=$enable_sse"]
	  ,
	  [AC_MSG_RESULT([don't know how to for compiler $CXX])]
	)
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
/* A "performance" warning for converting int to bool. */
# pragma warning(disable:4800)

/* POSIX get to deprecate POSIX things, not Microsoft. */
# define _CRT_NONSTDC_NO_WARNINGS
# define _CRT_SECURE_NO_WARNINGS

/* WSAAddressToStringA() is deprecated and we should apparently use
 * WSAAddressToStringW(), but we don't want wide character output so
 * that would mean we'd have to convert the result back to ASCII
 * which is a really stupid idea.
 */
# define _WINSOCK_DEPRECATED_NO_WARNINGS

/* Tell MSVC we want M_PI, etc defined. */
# define _USE_MATH_DEFINES

/* Tell MSVC we don't want max() and min() macros defined. */
# define NOMINMAX
#endif

/* Make the POSIX-like functions support large files.  MSVC needs this;
 * current mingw32 does too; mingw64 supports _FILE_OFFSET_BITS, which
 * AC_SYS_LARGEFILE should discover and enable automatically.
 */
#if defined _MSC_VER || \
    (defined __MINGW32__ && !defined _FILE_OFFSET_BITS)

# include <sys/stat.h>
# include <sys/types.h>
# include <io.h>
/* zlib.h uses off_t so we need to include it before we redefine off_t. */
# include <zlib.h>

// The default stat() and fstat() use 32-bit filesizes and time_t, so we need
// to arrange to use 64-bit variants instead.

# ifdef stat
// Break #undef lines to stop config.status from commenting them out.
#  undef \
	stat
# endif

// This hack is a problem is we ever want a method called "stat".
# ifdef _MSC_VER
// MSVC needs to call _stat64() instead of stat() and the struct which holds
// the information is `struct _stat64` instead of `struct stat` so we just
// use #define to replace both in one go.
#  define stat _stat64
# else
// Mingw32 has _stat64() but unhelpfully for our purposes, the struct is
// called __stat64 (with an extra underscore).  We hack around this by
// defining stat to __stat64 which sorts out the struct, and then using
// a second macro to forward function-like uses of __stat64() to
// _stat64().
#  define stat __stat64
#  define __stat64(PATH, STATBUF) _stat64((PATH), (STATBUF))
# endif

// We also want to use _fstat64() instead of fstat() but in this case we can
// use a function-like macro, so we could have a method called fstat so long
// as it didn't take two parameters.

# ifdef fstat
// Break #undef lines to stop config.status from commenting them out.
#  undef \
	fstat
# endif

# define fstat(FD, BUF) _fstat64(FD, BUF)

# ifdef lseek
// Break #undef lines to stop config.status from commenting them out.
#  undef \
	lseek
# endif

# ifdef off_t
// Break #undef lines to stop config.status from commenting them out.
#  undef \
	off_t
# endif

# define lseek(FD, OFF, WHENCE) _lseeki64(FD, OFF, WHENCE)
/* Redefine via a typedef so C++ code off_t(-1) works - it wouldn't if we did:
 * #define off_t long long
 */
typedef long long off_t_redefinition_typedef;
# define off_t off_t_redefinition_typedef

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

/* Signal we're building the library so it's OK to include headers such as
 * xapian/query.h directly.
 */
#define XAPIAN_LIB_BUILD 1

/* With Sun CC 5.13 (Studio 12.4) on Solaris 11.2, <math.h> seems to get
 * implicitly included somehow before <cmath>, and compilation fails due
 * to 'std::exception' colliding with 'struct exception'.  It's not clear
 * how to avoid this, so just define the same magic macro which <cmath> does
 * before it includes <math.h>.
 */
#if defined __SUNPRO_CC && !defined __MATHERR_RENAME_EXCEPTION
# define __MATHERR_RENAME_EXCEPTION
#endif
])

AC_SUBST([AM_CXXFLAGS])

dnl Restore CXXFLAGS to those the user specified or autoconf defaulted to.
CXXFLAGS=$save_CXXFLAGS

dnl Required for auto regeneration to work in a combined maintainer-mode tree.
: ${AUTOM4TE=autom4te}
AC_SUBST([AUTOM4TE])

dnl Libtool sets this (to yes|no|unknown) and we use it in xapian-config.
AC_SUBST([link_all_deplibs_CXX])

dnl Shared library extension.
module=no
eval "SHLIBEXT=$shrext_cmds"
AC_SUBST([SHLIBEXT])

dnl We want to be able to use GNU make % pattern rules in maintainer targets
dnl but automake warns these aren't portable, so we substitute the % to avoid
dnl this warning.
PERCENT='%'
AC_SUBST([PERCENT])

dnl Split up the version number into "MAJOR.MINOR.REVISION".
MAJOR_VERSION=`echo "$VERSION"|sed 's/\..*//'`
[MINOR_VERSION=`echo "$VERSION"|sed 's/[^.]*\.//;s/\..*//'`]
[REVISION=`echo "$VERSION"|sed 's/.*\.//;s/_.*$//'`]

dnl Convert the libs in $XAPIAN_LIBS to a CMake list.
XAPIAN_CMAKE_LIBS=`echo " $XAPIAN_LIBS" | sed 's/ -l/;/g;s/ //g'`
AC_SUBST([XAPIAN_CMAKE_LIBS])

XAPIAN_CMAKE_SHARED=
if test "$enable_shared" = yes; then
  XAPIAN_CMAKE_SHARED=ON
else
  XAPIAN_CMAKE_SHARED=OFF
fi
AC_SUBST([XAPIAN_CMAKE_SHARED])

LIBRARY_STATIC_EXT=.$libext
AC_SUBST([LIBRARY_STATIC_EXT])

if test "$win32" = yes; then
  LIBRARY_SHARED_IMP_EXT=$LIBRARY_STATIC_EXT
else
  LIBRARY_SHARED_IMP_EXT=
fi
AC_SUBST([LIBRARY_SHARED_IMP_EXT])

dnl We don't make use of maths function setting errno and if we tell the
dnl compiler that it can optimise them better in some cases.  Apparently
dnl most BSD-derived platforms don't set errno in this case anyway.
XAPIAN_TEST_CXXFLAGS([-fno-math-errno], [CXXFLAGS])

dnl **************************
dnl * Build the output files *
dnl **************************

AC_CONFIG_FILES([
 Makefile
 tests/Makefile
 docs/Makefile
 docs/doxygen_api.conf
 docs/doxygen_source.conf
 xapian-core.spec
 cmake/xapian-config.cmake
 cmake/xapian-config-version.cmake
 pkgconfig/xapian-core"$LIBRARY_VERSION_SUFFIX".pc:pkgconfig/xapian-core.pc.in
 ])
AC_CONFIG_FILES([makemanpage], [chmod +x makemanpage])
AC_CONFIG_FILES([tests/runtest], [chmod +x tests/runtest])
AC_CONFIG_FILES([tests/runsrv], [chmod +x tests/runsrv])
AC_CONFIG_FILES([tests/submitperftest], [chmod +x tests/submitperftest])
AC_CONFIG_FILES([tests/perftest/get_machine_info], [chmod +x tests/perftest/get_machine_info])
AC_CONFIG_FILES([xapian-config], [chmod +x xapian-config])
AC_OUTPUT

dnl There are no files generated by AC_OUTPUT in the following directories
dnl and we need to ensure they exist so that the rest of configure or make
dnl won't fail because they don't exist when srcdir != builddir.
if test yes = "$vpath_build" ; then
  for dir in include/xapian languages queryparser ; do
    AS_MKDIR_P(["$dir"])
  done
fi

dnl Generate include/xapian/version.h:

dnl MAIN_VERSION is VERSION without any _git123 suffix.
MAIN_VERSION="$MAJOR_VERSION.$MINOR_VERSION.$REVISION"
cxxcpp_flags=-I.
for backend in CHERT GLASS INMEMORY REMOTE ; do
  val=`eval echo "\\\$BUILD_BACKEND_${backend}_TRUE"`
  if test -z "$val" ; then
    cxxcpp_flags="$cxxcpp_flags -DXAPIAN_HAS_${backend}_BACKEND"
  fi
done

if test yes = "$enable_64bit_docid" ; then
  cxxcpp_flags="$cxxcpp_flags -DXAPIAN_DOCID_BASE_TYPE="`echo "$INT64_T"|sed 's/ /____/g'`
else
  cxxcpp_flags="$cxxcpp_flags -DXAPIAN_DOCID_BASE_TYPE="`echo "$INT32_T"|sed 's/ /____/g'`
fi

if test yes = "$enable_64bit_termcount" ; then
  cxxcpp_flags="$cxxcpp_flags -DXAPIAN_TERMCOUNT_BASE_TYPE="`echo "$INT64_T"|sed 's/ /____/g'`
else
  cxxcpp_flags="$cxxcpp_flags -DXAPIAN_TERMCOUNT_BASE_TYPE="`echo "$INT32_T"|sed 's/ /____/g'`
fi

if test yes = "$enable_64bit_termpos" ; then
  cxxcpp_flags="$cxxcpp_flags -DXAPIAN_TERMPOS_BASE_TYPE="`echo "$INT64_T"|sed 's/ /____/g'`
else
  cxxcpp_flags="$cxxcpp_flags -DXAPIAN_TERMPOS_BASE_TYPE="`echo "$INT32_T"|sed 's/ /____/g'`
fi

dnl Always ought to be 64 bit.
cxxcpp_flags="$cxxcpp_flags -DXAPIAN_TOTALLENGTH_TYPE="`echo "unsigned $INT64_T"|sed 's/ /____/g'`

dnl Make revision numbers in the API 64 bit for future-proofing.
cxxcpp_flags="$cxxcpp_flags -DXAPIAN_REVISION_TYPE="`echo "unsigned $INT64_T"|sed 's/ /____/g'`

dnl For GCC (and compilers which pretend to be GCC) and MSVC, we create checks
dnl for some ABI-incompatible configurations in the generated version.h.
dnl Such configurations may be specified via $CXXFLAGS so we should use this
dnl when generating version.h.  For these compilers, CXXCPP="$CXX -E" so it
dnl should understand all compiler options.
dnl
dnl For other compilers, we only pass $CPPFLAGS to $CXXCPP in case it's an
dnl external cpp which doesn't understand all options which might be in
dnl $CXXFLAGS.
case $GXX$msvc in
  *yes*)
    cxxcpp_flags="$cxxcpp_flags $CPPFLAGS $CXXFLAGS" ;;
  *)
    cxxcpp_flags="$cxxcpp_flags $CPPFLAGS" ;;
esac
rm -f include/xapian/version.h.tmp
dnl "\r" in sed is a GNUism, but we only need to remove it on MS Windows
dnl where we'll always have GNU sed, and other sed's will just interpret
dnl it as "r" (Solaris sed) or maybe literal "\r" which won't match.
dnl
dnl Use @@ around $MAIN_VERSION so we get " in the final output.
$CXXCPP $cxxcpp_flags\
	-DSTRING_VERSION="\"@@$MAIN_VERSION@@\""\
	-DMAJOR_VERSION="\"$MAJOR_VERSION\""\
	-DMINOR_VERSION="\"$MINOR_VERSION\""\
	-DREVISION="\"$REVISION\""\
	$dash_d_visibility\
	$srcdir/include/xapian/version_h.cc|\
	${SED-sed} '0,/const char \* dummy/d;/"/!d;s/^ *//;/^#/d;s/ *$//;s/" *,//;s/"//g;s/@@/"/g;s/  */ /g;s/ *,\r$//;s/ *,$//;s/____/ /g;s/\\\\$/\\/'\
	> include/xapian/version.h.tmp
dnl Only update the file if it has changed, so we don't alter the timestamp
dnl and cause lots of rebuilding needlessly.  However, the build system
dnl needs a timestamp to know when to regenerate version.h because version_h.cc
dnl has changed so we use a separate timestamp file.
touch include/xapian/version.h.timestamp
if cmp include/xapian/version.h.tmp include/xapian/version.h >/dev/null 2>&1
then
  rm include/xapian/version.h.tmp
else
  mv include/xapian/version.h.tmp include/xapian/version.h
fi
