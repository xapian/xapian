dnl Process this file with autoconf to produce a configure script.

dnl Need autoconf 2.50 or later for many features
dnl 2.54 needed for automake 1.6
dnl 2.57 fixes the annoying warnings from configure on FreeBSD
dnl 2.58 is needed by automake 1.8.5
dnl 2.59 was released the same day as 2.58 to fix a problem
AC_PREREQ(2.59)
dnl NB When updating the version for a release, update LIBRARY_VERSION_INFO
dnl below too.
AC_INIT(xapian-core, 0.8.3)dnl FIXME: bugreport addr as third argument
dnl Extract from the libtool info manual:
dnl
dnl Here are a set of rules to help you update your library version information:
dnl
dnl 1. Start with version information of `0:0:0' for each libtool library.
dnl
dnl 2. Update the version information only immediately before a public
dnl    release of your software.  More frequent updates are unnecessary,
dnl    and only guarantee that the current interface number gets larger
dnl    faster.
dnl
dnl 3. If the library source code has changed at all since the last
dnl    update, then increment REVISION (`C:R:A' becomes `C:r+1:A').
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
dnl Older libxapian versions:
dnl 0:0:0 0.7.2 (and 0.7.1 and 0.7.0 - 0.7.0 was the first libxapian version)
dnl 0:1:0 0.7.3
dnl 1:0:0 0.7.4 (Xapian::ESet now has reference counted internals)
dnl 1:1:1 0.7.5 (Xapian::{Term,Posting,Position}Iterator default ctors added)
dnl 2:0:0 0.8.0 (TradWeight ctor explicit; Xapian::Document::add_term() added)
dnl Older libxapianqueryparser versions:
dnl 0:0:0 0.7.2 (and 0.7.1 and 0.7.0)
dnl 0:1:0 0.7.3 (and 0.7.4)
dnl 0:2:0 0.7.5
dnl 1:0:0 0.8.0
dnl LIBRARY_VERSION_INFO:
dnl 3:0:0 0.8.1 (Xapian::Database::get_lastdocid added; QueryParser tries
dnl	       reparsing after stripping punctuation; several unused
dnl	       Error subclasses removed; assorted Iterator changes)
dnl 4:0:1 0.8.2 (Xapian::ESet::back and many other methods added)
dnl 4:1:1 0.8.3 No API changes
dnl and for the next release:
dnl 5:0:0 0.8.4 Some BM25Weight inline methods made non-inline.
LIBRARY_VERSION_INFO=4:1:1
AC_SUBST(LIBRARY_VERSION_INFO)

AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR(matcher/andpostlist.cc)

AC_CONFIG_HEADERS(config.h)

dnl Use libtool to manage our libraries
AC_PROG_LIBTOOL

dnl Add parameters for aclocal
dnl (This must come after AM_INIT_AUTOMAKE, since it modifies ACLOCAL)
ACLOCAL="$ACLOCAL $ACLOCAL_FLAGS"

dnl disable "maintainer only" rules by default
AM_MAINTAINER_MODE

dnl Checks for programs.
AC_PROG_CXX
AC_PROG_YACC

dnl Run tests using the C++ compiler.
AC_LANG_CPLUSPLUS

XAPIAN_LDFLAGS=
AC_SUBST(XAPIAN_LDFLAGS)

ldflags=
if test yesyes = "$GXX$enable_shared" ; then
  case $host_os in
    *mingw* | *cygwin*)
      ldflags="--enable-runtime-pseudo-reloc"
      if $CXX -Wl,$ldflags 2>&1 >/dev/null|grep -e $ldflags >/dev/null 2>&1; then
	dnl The error message contains the flag name - it must be a
	dnl complaint that the option is unrecognized (doing it this
	dnl way allows it to work regardless of the i18n in use):
	dnl ld: unrecognized option '--enable-runtime-pseudo-reloc'
	dnl
	dnl Can't use AC_DISABLE_SHARED after AC_PROG_LIBTOOL, but
	dnl this test needs to be after AC_PROG_LIBTOOL, so we can't
	dnl just disable the shared build automatically...
	AC_MSG_ERROR([ld version too old to support a shared build - configure with --disable-shared, or install binutils 2.13.90-20030111-1 or later])
      fi
      ldflags="-Wl,$ldflags"
    ;;
  esac
fi
dnl Only works for ldflags which can be specified anywhere on the link line.
AC_SUBST(ldflags)

dnl Some C++ compilers need a special switch to select ANSI C++ mode.  Do
dnl this early in configure so such settings are used for other tests.
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
    ANSI_CXXFLAGS="-AA" ;;
  cxx|*/cxx)
    dnl Select ANSI mode (strict_ansi is needed for ANSI iostream, as -std ansi
    dnl gives a pre-standard AT&T-compatible version).
    dnl If this is a problem, try "-std ansi -D__USE_STD_IOSTREAM"
    ANSI_CXXFLAGS="-std strict_ansi" ;;
esac

dnl Put any flags for ANSI mode in CXXFLAGS not AM_CXXFLAGS since AM_CXXFLAGS
dnl is only used in the Makefiles and we want to use such flags in configure
dnl tests.
test -n "$ANSI_CXXFLAGS" && CXXFLAGS="$ANSI_CXXFLAGS $CXXFLAGS"

dnl Make 'xapian-config --cxxflags' turn on any such flags for code linking
dnl with Xapian.
AC_SUBST(ANSI_CXXFLAGS)

dnl Check for time functions.
AC_CHECK_FUNCS(gettimeofday ftime)

dnl See if ftime returns void (as it does on mingw)
AC_MSG_CHECKING([return type of ftime])
if test $ac_cv_func_ftime = yes ; then
  AC_TRY_COMPILE([#include <sys/timeb.h>],
    [struct timeb tp; int i = ftime(&tp);],
    AC_MSG_RESULT(int),
    AC_MSG_RESULT(void)
    AC_DEFINE(FTIME_RETURNS_VOID, 1, [Define if ftime returns void]))
fi

dnl Check how to find the hostname: uname() in sys/utsname.h, or gethostname()
dnl Don't use default includes as inttypes.h is found by Compaq C but not C++
dnl so it causes all header probes to fail.
AC_CHECK_HEADERS(sys/utsname.h, [], [], [ ])
AC_CHECK_FUNCS(gethostname)

dnl mingw (for instance) lacks ssize_t
AC_CHECK_TYPE(ssize_t, long)

dnl Check for a stringstream implementation.  (If not present, we roll
dnl one ourselves).
AC_CHECK_HEADERS(sstream, [], [], [ ])
dnl Check for a new streams implementation.
AC_CHECK_HEADERS(streambuf, [], [], [ ])

dnl check for strcasecmp.
AC_CHECK_FUNCS([strcasecmp])

dnl check for socklen_t
TYPE_SOCKLEN_T

dnl Checks for perl (needed to regenerate some of the documentation)
AC_PATH_PROG(PERL, perl, [$MISSING perl])

dnl Make DATADIR available to the library
dnl This isn't currently used.  If we reenable it, it needs to be renamed
dnl to avoid a clash with a mingw header.
dnl AC_DEFINE_DIR(DATADIR, datadir, [Location of platform independent support files])

dnl Checks for dot.  (Diagrams in the documentation)
AC_PATH_PROG(DOT, dot)
if test -n "$DOT" ; then
  DOXYGEN_HAVE_DOT=YES
  DOXYGEN_DOT_PATH=`echo "$DOT" | sed "s/\/dot$//"`
else
  DOXYGEN_HAVE_DOT=NO
fi
AC_SUBST(DOXYGEN_HAVE_DOT)
AC_SUBST(DOXYGEN_DOT_PATH)

dnl Check for latex. (Needed to make some more of the documentation)
AC_PATH_PROG(LATEX, latex, [$MISSING latex])

dnl Check for makeindex. (Needed to make some more of the documentation)
AC_PATH_PROG(MAKEINDEX, makeindex, [$MISSING makeindex])

dnl Check for doxygen. (Needed to make some more of the documentation)
AC_PATH_PROG(DOXYGEN, doxygen, [$MISSING doxygen])

dnl Check for dvips. (Needed to make some more of the documentation)
AC_PATH_PROG(DVIPS, dvips, [$MISSING dvips])

dnl Check whether we need -ldl for dlsym() etc.
dnl No longer used...
dnl AC_TRY_LINK_FUNC(dlsym, ,
dnl	[AC_CHECK_LIB(dl, dlsym, [DL_LIBS="-ldl"])])
dnl AC_SUBST(DL_LIBS)

dnl Checks for header files.
AC_CHECK_HEADERS(fcntl.h limits.h, [], [], [ ])

dnl If valgrind is installed and new enough, we use it for leak checking in the
dnl testsuite.
AC_PATH_PROG(VALGRIND, valgrind, [])
if test -n "$VALGRIND" ; then
  dnl Check that the valgrind version installed supports the most recently
  dnl added client request which we use - VALGRIND_DO_LEAK_CHECK is ages old,
  dnl VALGRIND_COUNT_ERRORS was added at the same time as VALGRIND_COUNT_LEAKS
  AC_EGREP_CPP(yes,
    [#include <valgrind/memcheck.h>
     #ifdef VALGRIND_COUNT_LEAKS
     yes
     #endif
    ], [],
    [VALGRIND=])
fi

if test -n "$VALGRIND" ; then
  AC_DEFINE(HAVE_VALGRIND, 1,
	    [Define if valgrind is installed and it supports VALGRIND_COUNT_LEAKS])
fi

dnl Checks for library functions.
AC_FUNC_MEMCMP
AC_CHECK_FUNCS(strerror)

AC_MSG_CHECKING(for working snprintf)
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
  dnl ISO C90 semantics for the returned length when truncating.
  AC_RUN_IFELSE([
    AC_LANG_PROGRAM(
      [[#include <stdio.h>]],
      [[
	char buffer[4] = "abc";
	if ($func(buffer, 2, "%s", "XYZ") != 3) return 1;
	if (memcmp(buffer, "XYc", 4) != 0) return 1;
	if ($func(buffer, 2, "%x", 0x12) != 2) return 1;
	if (memcmp(buffer, "12c", 4) != 0) return 1
      ]]
    )],
    [ac_cv_func_snprintf="$func";break],
    [ac_cv_func_snprintf=no],
    [ac_cv_func_snprintf=unknown;break]
  )
done
AC_MSG_RESULT([$ac_cv_func_snprintf])
case "$ac_cv_func_snprintf" in
  no|unknown) ;; dnl be conservative when crosscompiling
  *)
    AC_DEFINE_UNQUOTED(SNPRINTF, [$ac_cv_func_snprintf],
       [Define to the name of a function implementing snprintf with ISO C90 semantics (if one exists)])
    ;;
esac

dnl ***************************
dnl * Select modules to build *
dnl ***************************

dnl Find out what we've been asked to build

dnl muscat36 defaults to off
AC_ARG_ENABLE(backend_muscat36,
[  --enable-backend-muscat36 Enable (old) Muscat 3.6 databases (default=no)],
[case "${enableval}" in
  yes) build_muscat36=true ;;
  no)  build_muscat36=false ;;
  *) AC_MSG_ERROR(bad value ${enableval} for --enable-backend-muscat36) ;;
esac],[build_muscat36=false])

AC_ARG_ENABLE(backend_quartz,
[  --enable-backend-quartz Enable the quartz database backend (default=yes)],
[case "${enableval}" in
  yes) build_quartz=true ;;
  no)  build_quartz=false ;;
  *) AC_MSG_ERROR(bad value ${enableval} for --enable-backend-quartz) ;;
esac],[build_quartz=true])

AC_ARG_ENABLE(backend_inmemory,
[  --enable-backend-inmemory Enable the inmemory database backend (default=yes)],
[case "${enableval}" in
  yes) build_inmemory=true ;;
  no)  build_inmemory=false ;;
  *) AC_MSG_ERROR(bad value ${enableval} for --enable-backend-inmemory) ;;
esac],[build_inmemory=true])

AC_ARG_ENABLE(backend_remote,
[  --enable-backend-remote Enable the remote database backend (default=yes)],
[case "${enableval}" in
  yes) build_remote=true ;;
  no)  build_remote=false ;;
  *) AC_MSG_ERROR(bad value ${enableval} for --enable-backend-remote) ;;
esac],[build_remote=true])
dnl check for fork - if we don't have it, the remote backend won't build
AC_CHECK_FUNCS(fork)
if test "$ac_cv_func_fork" = no ; then
  build_remote=false
else
  REMOTE_LDFLAGS=
  dnl Add -lsocket to $LDFLAGS if needed (eg Solaris)
  SAVE_LIBS="$LIBS"
  LIBS=
  AC_SEARCH_LIBS(socketpair, socket, [REMOTE_LDFLAGS="$LIBS"])
  LIBS="$SAVE_LIBS"
  if test "$ac_cv_func_socketpair" = no ; then
    build_remote=false
  else
    dnl Add -lnsl to LDFLAGS if needed (eg Solaris)
    SAVE_LIBS="$LIBS"
    LIBS=
    AC_SEARCH_LIBS(gethostbyname, nsl,
	    [REMOTE_LDFLAGS="$LIBS $REMOTE_LDFLAGS"],
	    build_remote=false)
    LIBS="$SAVE_LIBS"
    if test false != "$build_remote" ; then
      SAVE_LIBS="$LIBS"
      LIBS=
      AC_SEARCH_LIBS(gethostbyaddr, nsl,
	      [
	      case " $REMOTE_LDFLAGS " in
	      "* -lnsl *") ;;
	      *) REMOTE_LDFLAGS="$LIBS $REMOTE_LDFLAGS" ;;
	      esac
	      ],
	      build_remote=false)
      LIBS="$SAVE_LIBS"
    fi
  fi
  if test false != "$build_remote" ; then
    XAPIAN_LDFLAGS="$XAPIAN_LDFLAGS $REMOTE_LDFLAGS"
  fi
fi

AC_ARG_ENABLE(quiet,
[  --enable-quiet          Enable quiet building (default=no)],
[case "${enableval}" in
  yes) use_quiet=true ;;
  no)  use_quiet=false ;;
  *) AC_MSG_ERROR(bad value ${enableval} for --enable-quiet) ;;
esac], [use_quiet=false])

dnl Shut things up if we want to be quiet
if test true = "$use_quiet"; then
  LIBTOOL="$LIBTOOL --quiet"
fi

dnl Turn off compilation of anything that we don't have the requirements for

dnl Set conditionals to specify what we compile

AM_CONDITIONAL(BUILD_BACKEND_MUSCAT36, test true = "$build_muscat36")
AM_CONDITIONAL(BUILD_BACKEND_QUARTZ, test true = "$build_quartz")
AM_CONDITIONAL(BUILD_BACKEND_INMEMORY, test true = "$build_inmemory")
AM_CONDITIONAL(BUILD_BACKEND_REMOTE, test true = "$build_remote")
dnl Set a flag for AM when shared libraries are enabled.
dnl AM_CONDITIONAL(ENABLE_SHARED, test yes = "$enable_shared")

dnl Set defines to specify what we compile

if test true = "$build_muscat36"; then
AC_DEFINE(MUS_BUILD_BACKEND_MUSCAT36,,
  [Define if you want to build the (old) Muscat 3.6 database backend])
fi

if test true = "$build_quartz"; then
AC_DEFINE(MUS_BUILD_BACKEND_QUARTZ,,
  [Define if you want to build the quartz database backend])
fi

if test true = "$build_inmemory"; then
AC_DEFINE(MUS_BUILD_BACKEND_INMEMORY,,
  [Define if you want to build the inmemory database backend])
fi

if test true = "$build_remote"; then
  AC_DEFINE(MUS_BUILD_BACKEND_REMOTE,,
    [Define if you want to build the remote database backend])
fi

dnl See if we have fdatasync, and what libraries are needed for it.
SAVE_LIBS="$LIBS"
LIBS=
AC_SEARCH_LIBS(fdatasync, rt, [XAPIAN_LDFLAGS="$LIBS $XAPIAN_LDFLAGS"])
LIBS="$SAVE_LIBS"

AC_CHECK_FUNCS(fsync)

dnl HP-UX has pread and pwrite, but they don't work!  Apparently this problem
dnl manifests when largefile support is enabled, and we definitely want that
dnl so don't use pread or pwrite on HP-UX.
case "$host_os" in
  hpux*)
    AC_MSG_CHECKING([for pread])
    AC_MSG_RESULT([present but broken on $host_os])
    AC_MSG_CHECKING([for pwrite])
    AC_MSG_RESULT([present but broken on $host_os])
    ;;
  *)
    AC_CHECK_FUNCS(pread pwrite)
    ;;
esac

AC_CHECK_FUNCS(link)

dnl See if we want to use STLport
RJB_FIND_STLPORT

dnl *************************
dnl * Set debugging options *
dnl *************************

dnl Work out what debugging we want

use_debug=no
use_debug_paranoid=no
use_debug_verbose=no
use_debug_profile=no

AC_ARG_ENABLE(debug,
[  --enable-debug          Turn on debugging (no|partial|yes|full|profile)(default=no)],
[case "${enableval}" in
  full) use_debug_paranoid=yes ; use_debug=yes ; use_debug_verbose=yes ;;
  yes) use_debug_paranoid=yes ; use_debug=yes ;;
  partial) use_debug=yes ;;
  profile) use_debug_profile=yes ;;
  no) use_debug=no ;;
  *) AC_MSG_ERROR(bad value ${enableval} for --enable-debug) ;;
esac])

AC_ARG_ENABLE(debug-verbose,
[  --enable-debug-verbose  Turn on debugging messages (no|yes)(default=no)],
[case "${enableval}" in
  yes) use_debug_verbose=yes ;;
  no)  use_debug_verbose=no ;;
  *) AC_MSG_ERROR(bad value ${enableval} for --enable-debug-verbose) ;;
esac])

dnl Now set appropriate defines

if test yes = "$use_debug"; then
AC_DEFINE(MUS_DEBUG,,
  [Define if you want debugging to be enabled (will cause some slow down)])
fi

if test yes = "$use_debug_paranoid"; then
AC_DEFINE(MUS_DEBUG_PARANOID,,
  [Define if you want paranoid debugging to be enabled (will cause
  significant slow-down)])
fi

if test yes = "$use_debug_verbose"; then
AC_DEFINE(MUS_DEBUG_VERBOSE,,
  [Define if you want lots of debugging messages])
fi

if test yes = "$use_debug_profile"; then
AC_DEFINE(MUS_DEBUG_PROFILE,,
  [Define if you want code profiling messages])
fi

dnl ******************************
dnl * Set special compiler flags *
dnl ******************************

dnl extra flags for building with gcc
if test yes = "$GXX"; then
  dnl -Wno-long-long had appeared by GCC 2.95 and there's little likelihood
  dnl Xapian will build with any earlier version, so there's not much point
  dnl testing if -Wno-long-long is supported.
  AM_CXXFLAGS="$AM_CXXFLAGS -Wall -W -Wno-long-long -fno-gnu-keywords"
  dnl Automatically add -Werror if maintainer mode is enabled and we're using
  dnl GCC3 or newer.  We don't do this for older GCCs as GCC 2.95 issues
  dnl spurious warnings.
  if test x$USE_MAINTAINER_MODE = xyes; then
    case `$CXX --version` in
    [3-9]*.*|[12][0-9]*.*) 
      AM_CXXFLAGS="$AM_CXXFLAGS -Werror"
    esac
  fi
fi

dnl modify flags for building with stlport
STLPORT_CXXFLAGS=
if test yes = "$use_stlport"; then
  if test yes = "$GXX"; then
    STLPORT_CXXFLAGS="-nostdinc++"
  fi
  STLPORT_CXXFLAGS="$STLPORT_CXXFLAGS $STLPORT_INCLUDE"
  AM_CXXFLAGS="$STLPORT_CXXFLAGS $AM_CXXFLAGS"
  LIBS="$STLPORT_LIBS $LIBS"
fi
AC_SUBST(STLPORT_CXXFLAGS)

AC_SUBST(AM_CXXFLAGS)

dnl See if large file support is available
AC_SYS_LARGEFILE

APIDOC_SRC="`cd \"$srcdir\";echo include/xapian/*.h|sed 's!include/xapian/version.h!!;s!include!$T/include!g'`"
dnl find SOURCEDOC in 2 goes to avoid overflowing the 4096 character
dnl line length limit in some sed implementations
SOURCEDOC_CC_SRC="`cd \"$srcdir\";find . -name tests -prune -o -name \*.cc -print|sed 's!^\.!$T!'|tr '
' ' '`"
SOURCEDOC_H_SRC="`cd \"$srcdir\";find . -name tests -prune -o -name include -prune -o -name \*.h -print|sed '/^\.\/confdefs\.h$/d;s!^\.!$T!'|tr '
' ' '`"
AC_SUBST(APIDOC_SRC)
AC_SUBST(SOURCEDOC_CC_SRC)
AC_SUBST(SOURCEDOC_H_SRC)

dnl *******************************************************************
dnl * Make sure we can reference the top source and build directories *
dnl *******************************************************************

dnl The actual values of these are set in the makefile, so we protect
dnl against expansion here.  Contorted, but it works.
TOP_SRCDIR='$(top_srcdir)'
TOP_BUILDDIR='$(top_builddir)'

AC_SUBST(TOP_SRCDIR)
AC_SUBST(TOP_BUILDDIR)

dnl **************************
dnl * Build the output files *
dnl **************************

AC_CONFIG_FILES([
 Makefile
 testsuite/Makefile
 include/Makefile
 common/Makefile
 backends/Makefile
 backends/muscat36/Makefile
 backends/quartz/Makefile
 backends/multi/Makefile
 backends/inmemory/Makefile
 backends/net/Makefile
 languages/Makefile
 matcher/Makefile
 api/Makefile
 tests/Makefile
 docs/Makefile
 net/Makefile
 bin/Makefile
 getopt/Makefile
 extra/Makefile
 docs/doxygen_api_conf
 docs/doxygen_full_conf
 xapian.spec
 ])
AC_CONFIG_FILES([tests/runtest], [chmod +x tests/runtest])
AC_CONFIG_FILES([xapian-config], [chmod +x xapian-config])
AC_OUTPUT

MAJOR_VERSION="`echo \"$VERSION\"|sed 's/\..*//'`"
[MINOR_VERSION="`echo \"$VERSION\"|sed 's/[^.]*\.//;s/\..*//'`"]
REVISION="`echo \"$VERSION\"|sed 's/.*\.//'`"
dnl for portability, files run through $CXXCPP must have extension .c .cc or .C
tmpfile=_versionh.cc
rm -f $tmpfile
ln -s $srcdir/include/xapian/version.h.in $tmpfile
test -d include/xapian || mkdir include/xapian
dnl Use @ around $VERSION so we get " in the final output
$CXXCPP -DVERSION="\"@$VERSION@\"" -DMAJOR_VERSION="\"$MAJOR_VERSION\"" -DMINOR_VERSION="\"$MINOR_VERSION\"" -DREVISION="\"$REVISION\"" $tmpfile|grep '"'|${SED-sed} 's/^ *//;s/ *$//;/^#/d;s/" *,//;s/"//g;s/@/"/g;s/  */ /g;s/,$//' > include/xapian/version.h.tmp
rm -f $tmpfile
if cmp include/xapian/version.h.tmp include/xapian/version.h >/dev/null 2>&1
then
  rm include/xapian/version.h.tmp
else
  mv include/xapian/version.h.tmp include/xapian/version.h
fi
