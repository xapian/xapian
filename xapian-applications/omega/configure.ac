dnl Process this file with autoconf to produce a configure script.

AC_INIT([xapian-omega], [1.3.1], [http://xapian.org/bugs])
AC_PREREQ([2.63])
AM_INIT_AUTOMAKE([1.11 -Wportability tar-ustar no-dist-gzip dist-xz])

AC_CONFIG_SRCDIR([omega.cc])

AC_CONFIG_HEADERS([config.h])

AC_CONFIG_MACRO_DIR([m4])

LT_PREREQ([2.2.6])
LT_INIT

dnl -no-undefined can cause problems on some platforms, so only pass it
dnl on platforms where it is required in order to link a shared library at
dnl all (Windows is the main one).
NO_UNDEFINED=
if test unsupported = "$allow_undefined_flag" ; then
  NO_UNDEFINED=-no-undefined
fi
AC_SUBST(NO_UNDEFINED)

dnl Checks for programs.
AC_PROG_CXX

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

dnl Run tests using the C++ compiler.
AC_LANG_CPLUSPLUS

dnl Check for xapian-core.
XO_LIB_XAPIAN([], [], [xapian-config])

dnl We want XAPIAN_CXXFLAGS to be used for configure tests.
save_CXXFLAGS=$CXXFLAGS
CXXFLAGS="$CXXFLAGS $XAPIAN_CXXFLAGS"

dnl disable "maintainer only" rules by default
AM_MAINTAINER_MODE

dnl For _ftime64() on mingw we need to tell it we're happy to require
dnl MSVCRT 6.10 or higher, which isn't too onerous a requirement it seems.
case $host_os in
  *mingw*)
    AC_DEFINE([__MSVCRT_VERSION__], [0x0601], [Define on mingw to the minimum msvcrt version to assume])
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

dnl IRIX helpfully won't allow stdint.h to be included from C++ code,
dnl so we can't just use AC_CHECK_HEADERS.
AC_TRY_COMPILE([#include <stdint.h>],
  [uint32_t foo = 7; return (int)foo;],
  AC_DEFINE(HAVE_WORKING_STDINT_H, 1, [Define to 1 if you have the <stdint.h> header file and it can be used in C++ code.]))

dnl Check for headers.
AC_CHECK_HEADERS([strings.h sys/select.h], [], [], [ ])
AC_CHECK_HEADERS([netinet/in.h arpa/inet.h sys/time.h]dnl
		 [sys/resource.h sys/socket.h sys/sysctl.h vm/vm_param.h]dnl
		 [sys/vmmeter.h sys/sysmp.h sys/sysinfo.h sys/pstat.h],
		 [], [], [#include <sys/types.h>])

dnl libmagic used to detect MIME types for files we don't detect by extension.
AC_CHECK_HEADERS([magic.h])
if test $ac_cv_header_magic_h != yes ; then
  if test -r /etc/debian_version ; then
    pkg="libmagic-dev"
  else
    pkg="file-devel (or maybe libmagic-devl)"
  fi
  AC_MSG_ERROR([libmagic required but magic.h not found.  If you're installing from packages, you probably need to install a package called something like $pkg in order to be able to build $PACKAGE_NAME.])
fi
save_LIBS=$LIBS
LIBS=
AC_ARG_VAR(MAGIC_PREFIX, [Prefix for libmagic])
if test "$MAGIC_PREFIX"; then
AC_SEARCH_LIBS([magic_open], [magic], [], [
   MAGIC_LDFLAGS=-L$MAGIC_PREFIX/lib
   save_LDFLAGS=$LDFLAGS
   LDFLAGS="$LDFLAGS $MAGIC_LDFLAGS"
   AC_CHECK_LIB([magic], [magic_open], [], [
      MAGIC_LIBS=
      ])
   LDFLAGS=$save_LDFLAGS
   AC_SUBST([MAGIC_LDFLAGS])
 ])
else
AC_SEARCH_LIBS([magic_open], [magic])
fi
dnl magic_descriptor was added in file-4.22 released 2007-12-27.
AC_CHECK_FUNCS([magic_descriptor])
MAGIC_LIBS=$LIBS
LIBS=$save_LIBS
AC_SUBST([MAGIC_LIBS])

dnl mingw (for instance) lacks ssize_t
AC_CHECK_TYPE(ssize_t, int)
AC_TYPE_MODE_T
AC_TYPE_PID_T

dnl Check for time functions.
AC_FUNC_STRFTIME
AC_CHECK_FUNCS(gettimeofday ftime timegm)

dnl See if ftime() returns void (as it does on mingw).
if test $ac_cv_func_ftime = yes ; then
  AC_TRY_COMPILE([#include <sys/timeb.h>],
    [struct timeb tp; int i = ftime(&tp);],
    ,
    AC_DEFINE(FTIME_RETURNS_VOID, 1, [Define if ftime returns void]))
fi

dnl Solaris < 10 only has putenv().
dnl
dnl Microsoft have marked putenv() as deprecated, so we use _putenv_s() under
dnl MSVC to avoid deprecation warnings.  We probe here since mingw doesn't
dnl currently (v3.20) provide _putenv_s(), so we can't just use it conditional
dnl on __WIN32__ being defined.
AC_CHECK_FUNCS([setenv _putenv_s])

dnl Use for Unix permission checks.
AC_CHECK_FUNCS([getgrouplist])

dnl See if getgrouplist() takes gid_t * (e.g. Linux) or int * (e.g. OS X).
if test $ac_cv_func_getgrouplist = yes ; then
  AC_TRY_COMPILE([#include <grp.h>
#include <unistd.h>],
    [int g[10]; int n = 10; return getgrouplist("root", 1, g, &n) == -1;],
    AC_DEFINE([GETGROUPLIST_TAKES_INT_P], 1, [Define if getgrouplist takes int *]))
fi

dnl Check for lstat() (not available under mingw for example).
AC_CHECK_FUNCS(lstat)

dnl Add in portable replacement for mkdtemp() on platforms which lack it.
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

dnl omindex uses fork(), socketpair(), and setrlimit() to impose resource
dnl limits on filter programs.
AC_CHECK_FUNCS([mmap fork setrlimit sysmp pstat_getdynamic setpgid sigaction])

dnl -lxnet is needed on Solaris and apparently on HP-UX too.
AC_SEARCH_LIBS([socketpair], [xnet],
  [AC_DEFINE(HAVE_SOCKETPAIR, 1,
    [Define to 1 if you have the 'socketpair' function])])

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
    if test -f "/sw/etc/fink.conf" ; then
      dnl For fink on OS X.
      AC_CANONICAL_HOST
      case $host_os in
      darwin*)
	ICONV_LDFLAGS=-L/sw/lib
	SAVE_LDFLAGS=$LDFLAGS
	LDFLAGS="$LDFLAGS $ICONV_LDFLAGS"
	AC_CHECK_LIB(iconv, iconv, [], [
	  ICONV_LDFLAGS=
	  with_iconv=bad-$with_iconv
	])
	LDFLAGS=$SAVE_LDFLAGS ;;
      esac
    else
      with_iconv=bad-$with_iconv
    fi

    case $with_iconv in
    bad-yes) 
      AC_MSG_ERROR([iconv not found, but --with-iconv specified]) ;;
    bad-*)
      with_iconv=no ;;
    esac
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
	      $t in = const_cast<char *>("hello");
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

dnl Check for perl.  Needed for outlookmsg2html script.
dnl (And in maintainer-mode, also to make man pages from "--help" output, and
dnl to make the omegascript vim syntax mode.)
AC_PATH_PROG(PERL, perl, [])
test -z "$PERL" && AC_MSG_ERROR([perl is required])

if test x"$enable_documentation" = xyes; then
  dnl Check for help2man. (Needed to make man pages from "--help" output).
  AC_PATH_PROG(HELP2MAN, help2man, [])
  test -z "$HELP2MAN" && AC_MSG_ERROR([help2man is required to build documentation])

  dnl Check for rst2html. (Needed to make HTML from reStructuredText format)
  dnl Also look for rst2html.py, which archlinux reportedly installs it as.
  AC_PATH_PROGS(RST2HTML, [rst2html rst2html.py], [])
  test -z "$RST2HTML" && AC_MSG_ERROR([rst2html is required to build documentation])
fi

AC_ARG_VAR(PCRE_CONFIG, [Location of pcre-config])
AC_PATH_PROG(PCRE_CONFIG, pcre-config)
if test -z "$PCRE_CONFIG" ; then
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
      AC_MSG_ERROR([Can't find pcre-config, although the PCRE runtime library seems to be installed.  If you've installed PCRE from a package, you probably need to install an extra package called something like $pkg in order to be able to build $PACKAGE_NAME.])
    fi
  done
  AC_MSG_ERROR([Can't find pcre-config.  If the PCRE library is installed, you need to add PCRE_CONFIG=/path/to/pcre-config to your configure command.])
fi
PCRE_CFLAGS=`$PCRE_CONFIG --cflags`
PCRE_LIBS=`$PCRE_CONFIG --libs`
AC_SUBST(PCRE_CFLAGS)
AC_SUBST(PCRE_LIBS)

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
      dnl
      dnl -Wundef was supported by g++ 3.0 and since we now require GCC >= 3.1,
      dnl we can rely on it.  -fshow-column was present in GCC 3.1 too.
      dnl
      dnl All the other options were supported by g++ 2.95.
      AM_CXXFLAGS="$AM_CXXFLAGS -fshow-column -Wall -W -Wredundant-decls -Wpointer-arith -Wcast-qual -Wcast-align -Wno-long-long -Wformat-security -fno-gnu-keywords -Wundef -Woverloaded-virtual"

      dnl The exact format of g++ --version seems to change with almost every
      dnl minor release so use the preprocessor macros which should be more
      dnl robust.
      AC_MSG_CHECKING([for version of GCC])
      gxx_version=`echo __GNUC__.__GNUC_MINOR__.__GNUC_PATCHLEVEL__|$CXX -E -|sed '/^#/d;s/ //g'`
      AC_MSG_RESULT([$gxx_version])

      case $gxx_version in
      [[012]].* | 3.0.*)
	AC_MSG_ERROR([Xapian requires GCC 3.1 or later (you appear to have $gxx_version)])
	;;
      3.*|4.0.*) ;;
      *) dnl GCC >= 4.1
	case $gxx_version in
	4.1.*)
	  dnl -Wstrict-null-sentinel was new in 4.0.1, but for simplicity we
	  dnl only enable it for GCC >= 4.1.
	  dnl
	  dnl -Wshadow was supported by g++ 3.0 (though buggy then).  But it's
	  dnl also buggy in 4.0 (at least on Mac OS X) and warns if a parameter
	  dnl in a static method is the same as a member variable, so only
	  dnl enable it for GCC >= 4.1.
	  AM_CXXFLAGS="$AM_CXXFLAGS -Wstrict-null-sentinel -Wshadow" ;;
	4.2.*)
	  dnl -Wstrict-overflow was new in GCC 4.2.  Higher settings than 1
	  dnl have proved too noisy.
	  AM_CXXFLAGS="$AM_CXXFLAGS -Wstrict-null-sentinel -Wshadow -Wstrict-overflow=1" ;;
	4.[[345]].*)
	  dnl -Wlogical-op and -Wmissing-declarations (for C++) were added in
	  dnl GCC 4.3.
	  AM_CXXFLAGS="$AM_CXXFLAGS -Wstrict-null-sentinel -Wshadow -Wstrict-overflow=1 -Wlogical-op -Wmissing-declarations" ;;
	*) dnl GCC >= 4.6
	  dnl -Wdouble-promotion was added in GCC 4.6.
	  dnl
	  dnl -Winit-self was added in GCC 3.4, but for GCC < 4.7 g++ always
	  dnl warns for this case with -Wuninitialized (implied by -W).  We
	  dnl don't intend to use this idiom, so any instances are bugs we'd
	  dnl like to know about.
	  AM_CXXFLAGS="$AM_CXXFLAGS -Wstrict-null-sentinel -Wshadow -Wstrict-overflow=1 -Wlogical-op -Wmissing-declarations"
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

	if test no != "$enable_visibility"; then
	  dnl GCC doesn't support symbol visibility on all platforms (notably
	  dnl not Mac OS X or mingw).  Currently it seems to be ELF-only.
	  AC_MSG_CHECKING([if $CXX -fvisibility=hidden works])
	  if echo 'int foo() {return 42;}'|$CXX -Werror -fvisibility=hidden -c -oconftest.o -xc++ - >&AS_MESSAGE_LOG_FD 2>&AS_MESSAGE_LOG_FD ; then
	    AC_MSG_RESULT([yes])
	    AM_CXXFLAGS="$AM_CXXFLAGS -fvisibility=hidden"
	    dash_d_visibility=-DXAPIAN_ENABLE_VISIBILITY
	  else
	    AC_MSG_RESULT([no])
	  fi
	fi

	dnl Automatically add -Werror if maintainer mode is enabled and we're
	dnl using GCC 4.1 or newer.  We don't do this for older GCCs as GCC 2.95,
	dnl some GCC 3.x compilers, and Apple's GCC 4.0 issue spurious warnings.
	if test x$USE_MAINTAINER_MODE = xyes; then
	  AM_CXXFLAGS="$AM_CXXFLAGS -Werror"
	fi
	;;
      esac
      ;;
    *__clang__*)
      dnl Intel's compiler (since __clang__ not substituted):

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

      AM_CXXFLAGS="$AM_CXXFLAGS -Wall -W -Wredundant-decls -Wpointer-arith -Wcast-qual -Wcast-align -Wno-long-long -Wformat-security -fno-gnu-keywords -Wundef -Woverloaded-virtual"
      AM_CXXFLAGS="$AM_CXXFLAGS -Wshadow -Wstrict-overflow=1 -Winit-self -Wmissing-declarations"

      dnl Automatically add -Werror if maintainer mode is enabled.
      if test x$USE_MAINTAINER_MODE = xyes; then
	AM_CXXFLAGS="$AM_CXXFLAGS -Werror"
      fi
      ;;
  esac
fi

AC_SUBST(AM_CXXFLAGS)

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

dnl Restore CXXFLAGS to those the user specified or autoconf defaulted to.
CXXFLAGS=$save_CXXFLAGS

dnl Enable large file support if possible.
AC_SYS_LARGEFILE

dnl Required for auto regeneration to work in a combined maintainer-mode tree.
: ${AUTOM4TE=autom4te}
AC_SUBST([AUTOM4TE])

dnl **************************
dnl * Build the output files *
dnl **************************

AC_CONFIG_FILES([Makefile docs/Makefile xapian-omega.spec])
AC_CONFIG_FILES([makemanpage], [chmod +x makemanpage])
AC_CONFIG_FILES([outlookmsg2html], [chmod +x outlookmsg2html])
AC_OUTPUT
