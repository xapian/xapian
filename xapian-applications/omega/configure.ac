dnl Process this file with autoconf to produce a configure script.

m4_define([project_version], [1.5.0])
AC_INIT([xapian-omega], project_version, [https://xapian.org/bugs])
AC_PREREQ([2.63])
AM_INIT_AUTOMAKE([1.13 -Wportability subdir-objects tar-ustar no-dist-gzip dist-xz])

AC_CONFIG_SRCDIR([omega.cc])

AC_CONFIG_HEADERS([config.h])

AC_CONFIG_MACRO_DIRS([m4])

dnl The AM_ICONV macro requires a stack of macros including AC_LIB_PROG_LD,
dnl which is just a copy of libtool's LT_PATH_LD with the name changed.
dnl We will already have run LT_PATH_LD, so make AC_LIB_PROG_LD a no-op
dnl as LT_PATH_LD will already have set the with_gnu_ld variable.
AC_DEFUN([AC_LIB_PROG_LD], [])

LT_PREREQ([2.2.6])
LT_INIT

dnl We need to specify the argument types for builtin functions, or else
dnl AC_CHECK_DECLS fails to detect them when the compiler is clang.
AC_CHECK_DECLS([__builtin_add_overflow(int, int, int*),
    __builtin_mul_overflow(int, int, int*)], [], [], [ ])

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
AC_LANG([C++])

dnl Enable large file support if possible.
AC_SYS_LARGEFILE

dnl Test if the compiler works with $1 added to CXXFLAGS; if it does, add $1 to
dnl variable $2.  If the test passes, also do $3; if it fails, also do $4.
AC_DEFUN([XAPIAN_TEST_CXXFLAGS],
  [
  XTC_save_CXXFLAGS=$CXXFLAGS
  CXXFLAGS="$CXXFLAGS $1"
  AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[]], [[]])],
		    [CXXFLAGS=$XTC_save_CXXFLAGS
		     $2="${$2} $1"
		     $3],
		    [CXXFLAGS=$XTC_save_CXXFLAGS
		     $4])
  ])

dnl Check for xapian-core.
XO_LIB_XAPIAN([], [],
    [xapian-config]regexp(project_version,
			  [^\([0-9]*\.[0-9]*[13579]\)\..*$], [-\1]))
XO_REQUIRE([1.4.0])

dnl We want XAPIAN_CXXFLAGS to be used for configure tests.
save_CXXFLAGS=$CXXFLAGS
CXXFLAGS="$CXXFLAGS $XAPIAN_CXXFLAGS"

AC_CHECK_DECLS([__builtin_bswap32(uint32_t)], [], [],
	       [#include <stdint.h>])
AC_CHECK_DECLS([_byteswap_ulong], [], [],
	       [#include <stdlib.h>])

dnl Check for time functions.
AC_FUNC_STRFTIME
AC_CHECK_FUNCS([clock_gettime sleep nanosleep gettimeofday ftime])
AC_CHECK_FUNCS([gmtime_r timegm])

case $host_os in
  *mingw*)
    dnl For _ftime64() on mingw we need to tell it we're happy to require
    dnl MSVCRT 6.10 or higher, which isn't too onerous a requirement it seems.
    AC_DEFINE([__MSVCRT_VERSION__], [0x0601], [Define on mingw to the minimum msvcrt version to assume])
    AC_DEFINE([MINGW_HAS_SECURE_API], [1], [Define on mingw to get _s suffixed "secure" functions declared in headers])
    ;;
esac

AC_ARG_ENABLE(documentation,
  [AS_HELP_STRING([--enable-documentation], [enable make rules to rebuild documentation [default=maintainer-mode]])],
  [case ${enableval} in
    yes|no) ;;
    *) AC_MSG_ERROR([bad value ${enableval} for --enable-documentation]) ;;
  esac])
test -z "$enable_documentation" && enable_documentation=$USE_MAINTAINER_MODE
AM_CONDITIONAL(DOCUMENTATION_RULES, test x"$enable_documentation" = xyes)
AM_CONDITIONAL(MAINTAINER_NO_DOCS, test x"$USE_MAINTAINER_MODE$enable_documentation" = xyesno)

vpath_build=no
if test "`pwd`" != "`cd $srcdir;pwd`" ; then
  vpath_build=yes
fi
AM_CONDITIONAL([VPATH_BUILD], [test yes = "$vpath_build"])

dnl Check for headers.
AC_CHECK_HEADERS([strings.h sys/select.h], [], [], [ ])
AC_CHECK_HEADERS([sys/time.h]dnl
		 [sys/resource.h sys/socket.h sys/sysctl.h vm/vm_param.h]dnl
		 [sys/vmmeter.h sys/sysmp.h sys/sysinfo.h sys/pstat.h],
		 [], [], [#include <sys/types.h>])

dnl Check for zlib.h.
AC_CHECK_HEADERS([zlib.h], [], [
  AC_MSG_ERROR([zlib.h not found])
  ], [ ])

dnl Check for zlibVersion in -lz.
SAVE_LIBS=$LIBS
LIBS=
dnl mingw build needs -lzlib or -lzdll.
AC_SEARCH_LIBS([zlibVersion], [z zlib zdll], [], [
  AC_MSG_ERROR([zlibVersion() not found in -lz, -lzlib, or -lzdll (you  may need to install the zlib1g-dev or zlib-devel package)])
  ])
ZLIB_LIBS=$LIBS
AC_SUBST([ZLIB_LIBS])
LIBS=$SAVE_LIBS

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

dnl libmagic used to detect MIME types for files we don't detect by extension.
AC_CHECK_HEADERS([magic.h])
if test $ac_cv_header_magic_h != yes ; then
  if test -r /etc/debian_version ; then
    pkg="libmagic-dev"
  else
    pkg="file-devel (or maybe libmagic-devel)"
  fi
  AC_MSG_ERROR([libmagic required but magic.h not found.  If you're installing from packages, you probably need to install a package called something like $pkg in order to be able to build $PACKAGE_NAME.])
fi

dnl On mingw there are several dependent libraries, so AC_CHECK_LIB would
dnl fail - we'd have to use LT_OUTPUT and then perform the test link with
dnl libtool, but we've checked for magic.h, so checking the library would
dnl not really gain us much - we know it is -lmagic.
MAGIC_LIBS=-lmagic
AC_SUBST([MAGIC_LIBS])

dnl See if ftime returns void (as it does on mingw)
AC_MSG_CHECKING([return type of ftime])
if test $ac_cv_func_ftime = yes ; then
  AC_COMPILE_IFELSE(
    [AC_LANG_PROGRAM([[#include <sys/timeb.h>]],
		     [[struct timeb tp; int i = ftime(&tp);]])],
    [AC_MSG_RESULT(int)],
    [AC_MSG_RESULT(void)
     AC_DEFINE([FTIME_RETURNS_VOID], [1], [Define if ftime returns void])])
fi

dnl mingw (for instance) lacks ssize_t
AC_TYPE_SSIZE_T

AC_TYPE_PID_T

AC_TYPE_MODE_T

dnl Use for Unix permission checks.
AC_CHECK_FUNCS([getgrouplist])

dnl See if getgrouplist() takes gid_t * (e.g. Linux) or int * (e.g. OS X).
if test $ac_cv_func_getgrouplist = yes ; then
  AC_COMPILE_IFELSE(
    [AC_LANG_PROGRAM([[#include <grp.h>
#include <unistd.h>]],
		     [[int g[10]; int n = 10; return getgrouplist("root", 1, g, &n) == -1;]])],
    [AC_DEFINE([GETGROUPLIST_TAKES_INT_P], 1, [Define if getgrouplist takes int *])],
    [])
fi

dnl Check for lstat() (not available under mingw for example).
AC_CHECK_FUNCS(lstat)

dnl Add in portable replacement for mkdtemp() on platforms which lack it.
AC_CHECK_FUNCS(mkdtemp)
AM_CONDITIONAL(NEED_MKDTEMP, [test yes != "$ac_cv_func_mkdtemp"])

dnl Add in portable replacement for strptime() on platforms which lack it.
AC_CHECK_FUNCS(strptime)
AM_CONDITIONAL(NEED_STRPTIME, [test yes != "$ac_cv_func_strptime"])

AC_CHECK_FUNCS([posix_fadvise])

dnl omindex uses fork(), socketpair(), setrlimit() and alarm() to impose resource
dnl limits on filter programs and workers.
AC_CHECK_FUNCS([mmap fork setrlimit sysmp pstat_getdynamic setpgid sigaction alarm])

dnl -lxnet is needed on Solaris and apparently on HP-UX too.
AC_SEARCH_LIBS([socketpair], [xnet],
  [AC_DEFINE(HAVE_SOCKETPAIR, 1,
    [Define to 1 if you have the 'socketpair' function])])

dnl fnmatch() is used to match ignore pattern.
AC_CHECK_FUNCS([fnmatch])

OMINDEX_MODULES=""

dnl check if poppler is available.
PKG_CHECK_MODULES([POPPLER], [poppler-glib >= 0.16], [
    AC_DEFINE(HAVE_POPPLER, 1, [Define HAVE_POPPLER if the library is available])
    OMINDEX_MODULES="$OMINDEX_MODULES omindex_poppler"],[ ])

dnl check if libe-book is available.
PKG_CHECK_MODULES([LIBEBOOK], [libe-book-0.1,librevenge-0.0,librevenge-generators-0.0,librevenge-stream-0.0], [
    AC_DEFINE(HAVE_LIBEBOOK, 1, [Define HAVE_LIBEBOOK if the library is available])
    OMINDEX_MODULES="$OMINDEX_MODULES omindex_libebook"],[ ])

dnl check if libetonyek is available.
PKG_CHECK_MODULES([LIBETONYEK], [libetonyek-0.1,librevenge-0.0,librevenge-generators-0.0,librevenge-stream-0.0], [
    AC_DEFINE(HAVE_LIBETONYEK, 1, [Define HAVE_LIBETONYEK if the library is available])
    OMINDEX_MODULES="$OMINDEX_MODULES omindex_libetonyek"],[ ])

dnl check if libtesseract is available.
PKG_CHECK_MODULES([TESSERACT], [tesseract, lept], [
    AC_DEFINE(HAVE_TESSERACT, 1, [Define HAVE_TESSERACT if the library is available])
    OMINDEX_MODULES="$OMINDEX_MODULES omindex_tesseract"],[ ])

dnl check if gmime is available.
PKG_CHECK_MODULES([GMIME], [gmime-2.6, glib-2.0], [
    AC_DEFINE(HAVE_GMIME, 1, [Define HAVE_GMIME if the library is available])
    OMINDEX_MODULES="$OMINDEX_MODULES omindex_gmime"],[ ])

dnl check if libarchive is available.
PKG_CHECK_MODULES([LIBARCHIVE], [libarchive], [
    AC_DEFINE(HAVE_LIBARCHIVE, 1, [Define HAVE_LIBARCHIVE if the library is available])
    OMINDEX_MODULES="$OMINDEX_MODULES omindex_libarchive"],[ ])

dnl check if libabw is available.
PKG_CHECK_MODULES([LIBABW], [libabw-0.1,librevenge-0.0,librevenge-generators-0.0,librevenge-stream-0.0], [
    AC_DEFINE(HAVE_LIBABW, 1, [Define HAVE_LIBABW if the library is available])
    OMINDEX_MODULES="$OMINDEX_MODULES omindex_libabw"],[ ])

dnl check if libcdr is available.
PKG_CHECK_MODULES([LIBCDR], [libcdr-0.1,librevenge-0.0,librevenge-generators-0.0,librevenge-stream-0.0], [
    AC_DEFINE(HAVE_LIBCDR, 1, [Define HAVE_LIBCDR if the library is available])
    OMINDEX_MODULES="$OMINDEX_MODULES omindex_libcdr"],[ ])

dnl check if libextractor is available.
PKG_CHECK_MODULES([LIBEXTRACTOR], [libextractor], [
    AC_DEFINE(HAVE_LIBEXTRACTOR, 1, [Define HAVE_LIBEXTRACTOR if the library is available])
    OMINDEX_MODULES="$OMINDEX_MODULES omindex_libextractor"],[ ])

AC_SUBST([OMINDEX_MODULES])

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

AC_CHECK_MEMBERS([struct tm.tm_gmtoff], [], [], [#include <time.h>])

AC_ARG_WITH(iconv,
[AS_HELP_STRING([--with-iconv], [force use of iconv (error if not found)])
AS_HELP_STRING([--without-iconv], [don't use iconv])],
  [with_iconv=$withval], [with_iconv=auto])

if test no != "$with_iconv" ; then
  dnl AM_ICONV modifies CPPFLAGS if it succeeds which is undesirable (configure
  dnl shouldn't modify user variables like CPPFLAGS) so save and restore
  dnl CPPFLAGS around it.  We then add INCICONV where needed instead.
  save_CPPFLAGS=$CPPFLAGS
  AM_ICONV
  CPPFLAGS=$save_CPPFLAGS
  AC_SUBST([INCICONV])
  if test yes = "$with_iconv" && test yes != "$am_cv_func_iconv" ; then
    AC_MSG_ERROR([iconv not found, but --with-iconv specified])
  fi
fi

dnl Check processor endianness.
AC_C_BIGENDIAN

dnl Check for perl.  Needed for outlookmsg2html script.
dnl (And in maintainer-mode, also to make man pages from "--help" output, and
dnl to make the omegascript vim syntax mode.)
AC_PATH_PROG(PERL, perl, [])
AC_ARG_VAR([PERL], [Perl interpreter])
test -z "$PERL" && AC_MSG_ERROR([perl is required])

if test x"$enable_documentation" = xyes; then
  dnl Check for help2man. (Needed to make man pages from "--help" output).
  AC_PATH_PROG(HELP2MAN, help2man, [])
  AC_ARG_VAR([HELP2MAN], [help2man man page generator])
  test -z "$HELP2MAN" && AC_MSG_ERROR([help2man is required to build documentation])

  dnl Check for rst2html. (Needed to make HTML from reStructuredText format)
  dnl Also look for rst2html.py, which archlinux reportedly installs it as.
  AC_PATH_PROGS(RST2HTML, [rst2html rst2html.py], [])
  AC_ARG_VAR([RST2HTML], [reST to HTML convertor])
  test -z "$RST2HTML" && AC_MSG_ERROR([rst2html is required to build documentation])
fi

pcre_runtime_installed() {
  dnl Simple check to see if the problem is likely to be that we're using a
  dnl "packaged" pcre but only have the runtime package installed.
  for sfx in '' 32 64 ; do
    set /usr/lib$sfx/libpcre*.so.*
    if test "/usr/lib$sfx/libpcre?.so.*" != "$1" ; then
      if test -r /etc/debian_version ; then
	pkg="libpcre3-dev"
      else
	pkg="pcre-devel"
      fi
      AS_SET_STATUS([0])
    fi
  done
  AS_SET_STATUS([1])
}

AC_ARG_VAR(PCRE_CONFIG, [Location of pcre-config])
if test -z "$PCRE_CONFIG" ; then
  AC_ARG_VAR(PKG_CONFIG, [Location of pkg-config])
  AC_PATH_TOOL(PKG_CONFIG, [pkg-config])
  if test -n "$PKG_CONFIG" ; then
    pcre_try="$PKG_CONFIG libpcre"
    if $pcre_try 2>/dev/null ; then
      PCRE_CONFIG=$pcre_try
    else
      if pcre_runtime_installed ; then
	AC_MSG_ERROR(['$pcre_try' failed, but the PCRE runtime library seems to be installed.  If you've installed PCRE from a package, you probably need to install an extra package called something like $pkg in order to be able to build $PACKAGE_NAME.])
      else
	AC_MSG_ERROR(['$pcre_try' failed.  If the PCRE library is installed, you need to add PCRE_CONFIG=/path/to/pcre-config to your configure command.])
      fi
    fi
  else
    dnl Fall back to looking for pcre-config, but that's not installed on PATH
    dnl with the platform tuple prefixed (at least under MXE), so preferring
    dnl 'pkg-config libpcre' works better when cross-compiling.
    AC_PATH_TOOL(PCRE_CONFIG, [pcre-config])
    if test -z $PCRE_CONFIG ; then
      if pcre_runtime_installed ; then
	AC_MSG_ERROR([Can't find pcre-config, although the PCRE runtime library seems to be installed.  If you've installed PCRE from a package, you probably need to install an extra package called something like $pkg in order to be able to build $PACKAGE_NAME.])
      else
	AC_MSG_ERROR([Can't find pcre-config.  If the PCRE library is installed, you need to add PCRE_CONFIG=/path/to/pcre-config to your configure command.])
      fi
    fi
  fi
fi
PCRE_CFLAGS=`$PCRE_CONFIG --cflags`
PCRE_LIBS=`$PCRE_CONFIG --libs`
AC_SUBST(PCRE_CFLAGS)
AC_SUBST(PCRE_LIBS)

AC_MSG_CHECKING([for /dev/stdin])
if test -c /dev/stdin && (echo hello|$GREP hello /dev/stdin >&AS_MESSAGE_LOG_FD 2>&1); then
  AC_MSG_RESULT([yes])
  AC_DEFINE([HAVE_DEV_STDIN], [1], [Define to 1 if you have the '/dev/stdin' special file])
else
  AC_MSG_RESULT([no])
fi

dnl ******************************
dnl * Set special compiler flags *
dnl ******************************

dnl Set flags to control warnings (enable more, or disable annoying ones).
if test yes = "$GXX"; then
  dnl Intel's C++ compiler and clang both lie and define __GNUC__, so check which
  dnl we actually have, as neither is really 100% compatible.
  case `echo __INTEL_COMPILER __clang__|$CXX -E - 2>&AS_MESSAGE_LOG_FD|grep -v '^#'` in
    *__INTEL_COMPILER*__clang__*)
      dnl GCC (since neither substituted):

      dnl The exact format of g++ --version seems to change with almost every
      dnl minor release so use the preprocessor macros which should be more
      dnl robust.
      AC_MSG_CHECKING([for version of $CXX])
      gxx_version=`echo __GNUC__.__GNUC_MINOR__.__GNUC_PATCHLEVEL__|$CXX -E -|sed '/^#/d;s/ //g'`
      AC_MSG_RESULT([GCC $gxx_version])

      dnl -Wundef was supported by g++ 3.0.
      dnl
      dnl -Wlogical-op and -Wmissing-declarations (for C++) were added in
      dnl GCC 4.3.
      dnl
      dnl -Wdouble-promotion was added in GCC 4.6.
      dnl
      dnl All the other options were supported by g++ 2.95.
      AM_CXXFLAGS="$AM_CXXFLAGS -Wall -W -Wredundant-decls -Wpointer-arith -Wcast-qual -Wcast-align -Wformat-security -fno-gnu-keywords -Wundef -Woverloaded-virtual -Wstrict-null-sentinel -Wshadow -Wstrict-overflow=1 -Wlogical-op -Wmissing-declarations -Wdouble-promotion"

      case $gxx_version in
      [[0123]].*|4.[[0-7]].*)
	AC_MSG_ERROR([Xapian requires GCC 4.8 or later])
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

      dnl These options all work at least as far back as clang++ 3.0:
      AM_CXXFLAGS="$AM_CXXFLAGS -Wall -W -Wredundant-decls -Wpointer-arith -Wcast-qual -Wcast-align -Wformat-security -fno-gnu-keywords -Wundef -Woverloaded-virtual -Wshadow -Wstrict-overflow=1 -Wmissing-declarations -Winit-self"

      dnl Automatically add -Werror if maintainer mode is enabled.
      if test x$USE_MAINTAINER_MODE = xyes; then
	AM_CXXFLAGS="$AM_CXXFLAGS -Werror"
      fi
      ;;
  esac
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
    dnl 20201 "Memory leak is detected" which triggers for "return new Foo;"!
    AM_CXXFLAGS="$AM_CXXFLAGS +w +wlint +W2340,2401,3348,4255,4273,20201" ;;
  */sunCC|*/CC)
    dnl Sun's C++ compiler warns about functions failing to return a value even
    dnl when the function ends with a "throw" statement.  That's just unhelpful
    dnl noise, and adding a dummy "return" after "throw" seems a worse solution
    dnl than simply disabling this warning - any genuinely missing return
    dnl statements will get reported by compilers with superior warning
    dnl machinery.
    XAPIAN_TEST_CXXFLAGS([-erroff=voidretw], [AM_CXXFLAGS])
    ;;
  esac
fi

AH_BOTTOM(
[/* Disable stupid MSVC "performance" warning for converting int to bool. */
#ifdef _MSC_VER
# pragma warning(disable:4800)
#endif

/* Default to enabling _FORTIFY_SOURCE at level 2.  It shouldn't cause a
 * problem to define it where it's not supported.
 *
 * Check if _FORTIFY_SOURCE is already defined to allow the user to override
 * our choice with "./configure CPPFLAGS=-D_FORTIFY_SOURCE=0" or "...=1".
 */
#if defined __GNUC__ && !defined _FORTIFY_SOURCE
# define _FORTIFY_SOURCE 2
# ifdef __MINGW32__
/* Both mingw32 and mingw-w64 define __MINGW32__. */
#  include <stddef.h>
#  ifdef __MINGW64_VERSION_MAJOR
/* Enabling _FORTIFY_SOURCE on newer mingw-w64 requires linking with -lssp so
 * we simply don't automatically enable it there.
 */
#   undef _FORTIFY_SOURCE
#  endif
# endif
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

AC_SUBST([AM_CXXFLAGS])

dnl Restore CXXFLAGS to those the user specified or autoconf defaulted to.
CXXFLAGS=$save_CXXFLAGS

dnl Required for auto regeneration to work in a combined maintainer-mode tree.
: ${AUTOM4TE=autom4te}
AC_SUBST([AUTOM4TE])

dnl **************************
dnl * Build the output files *
dnl **************************

AC_CONFIG_FILES([
 Makefile
 docs/Makefile
 xapian-omega.spec
 ])
dnl Allow AC_CONFIG_FILES with chmod +x without having to repeat the filename.
AC_DEFUN([SCRIPT_TEMPLATE], [AC_CONFIG_FILES([$1], [chmod +x "$1"])])
SCRIPT_TEMPLATE([makemanpage])
SCRIPT_TEMPLATE([mhtml2html])
SCRIPT_TEMPLATE([outlookmsg2html])
SCRIPT_TEMPLATE([rfc822tohtml])
SCRIPT_TEMPLATE([vcard2text])
AC_OUTPUT
