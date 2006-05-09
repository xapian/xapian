dnl Process this file with autoconf to produce a configure script.

dnl Need autoconf 2.50 or later for many features
dnl 2.54 needed for automake 1.6
dnl 2.57 fixes the annoying warnings from configure on FreeBSD
dnl 2.58 is needed by automake 1.8.5
dnl 2.59 was released the same day as 2.58 to fix a problem
AC_PREREQ(2.59)
dnl NB When updating the version for a release, update LIBRARY_VERSION_INFO
dnl below too.
AC_INIT(xapian-core, 0.9.5)dnl FIXME: bugreport addr as third argument
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
dnl 5:0:0 0.8.4 BM25Weight ctor parameters reordered
dnl 5:1:0 0.8.5 No API changes
dnl 6:0:0 0.9.0 QueryParser API overhauled, etc
dnl 7:0:0 0.9.1 Query::get_terms_end() now inline in header
dnl 8:0:0 0.9.2 QueryParser::parse_query has new optional argument
dnl 9:0:1 0.9.3 Added xapian_version_string(), etc
dnl 10:0:0 0.9.4 Fixed typo in name of xapian_revision()
dnl 11:0:0 0.9.5 Document::termlist_end() now inline in header, etc
LIBRARY_VERSION_INFO=11:0:0
AC_SUBST(LIBRARY_VERSION_INFO)

dnl Check the build directory doesn't contain a space, so we die early with
dnl a helpful error.
case `pwd` in
*' '*)
   AC_MSG_ERROR([You can't build in a directory whose path contains a space])
   ;;
*) ;;
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
*) ;;
esac

dnl Check the prefix to install in doesn't contain a space, so we die early with
dnl a helpful error.
case $prefix in
*' '*)
   AC_MSG_ERROR([You can't install in a directory whose path contains a space])
   ;;
*) ;;
esac

dnl Note if the user specified a particular C++ compiler so we can give a more
dnl appropriate error message if we can't link a simple C++ program.
original_CXX=
if test -n "$CXX" ; then
  original_CXX="CXX=$CXX"
elif test -n "$CCC" ; then
  original_CXX="CCC=$CCC"
fi

dnl Need 1.5 for AM_CXXFLAGS, etc; also so generated files in EXTRA_DIST work
dnl Need 1.6.3 for tests/internaltest to build with Solaris make
dnl 1.8.5 contains a lot of fixes over 1.6.3
AM_INIT_AUTOMAKE([1.8.5])
AC_CONFIG_SRCDIR(matcher/andpostlist.cc)

AC_CONFIG_HEADERS(config.h)

dnl Use libtool to manage our libraries
AC_PROG_LIBTOOL

dnl A standard "gotcha" for mingw and cygwin users is to not set up their
dnl PATH correctly, so that MSDOS FIND.EXE is found before Unix find.  Help
dnl them out by checking for this condition, rather than letting libtool
dnl fail in obscure ways.  NB check the *BUILD* OS, not the host one!
case $build_os in
  *mingw* | *cygwin* | pw32*)
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
***   http://www.cygwin.com/faq/faq_3.html#SEC41
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

dnl For reasons which are beyond me, if autoconf can't find a C++ compiler
dnl it will set CXX to g++ (which obviously won't work) rather than actually
dnl telling the user that it couldn't find a C++ compiler and telling them
dnl to install one or set CXX if there's one configure failed to find.
dnl It's probably worthwhile checking that the C++ compiler actually works
dnl anyway!
cxx_works=no
if test -n "$CXX" ; then
  AC_LANG_CPLUSPLUS
  AC_MSG_CHECKING([whether the C++ compiler works])
  AC_TRY_RUN([int main() {}],
	     cxx_works=yes,
	     cxx_works=no,
	     AC_TRY_LINK([], [], cxx_works=yes, cxx_works=no))
  AC_MSG_RESULT($cxx_works)
fi
if test no = "$cxx_works" ; then
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

dnl We want a non-cross-compiling C compiler for building lemon with.
if test -z "$CC_FOR_BUILD" ; then
  AC_PROG_CC
  if test yes = "$cross_compiling"; then
    CC_FOR_BUILD=cc
  else
    CC_FOR_BUILD="$CC"
  fi
  dnl Unset CC to make sure we don't accidentally use it.
  unset CC
fi
AC_ARG_VAR(CC_FOR_BUILD, [C compiler command for native compilation (needed to compile build tools during cross-builds)])

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

WARNING_CXXFLAGS=
AC_MSG_CHECKING([for $CXX option to enable ANSI C++ mode])
if test yes = "$GXX"; then
  AC_MSG_RESULT([none required])
else
  dnl Some C++ compilers need a special switch to select ANSI C++ mode.  Do
  dnl this early in configure so such settings are used for other tests.
  case $CXX in
    aCC|*/aCC)
      dnl Select ANSI mode for HP's aCC, which gives us the std:: namespace,
      dnl Koenig lookup, and ANSI scoping for the variable i in:
      dnl   for (int i = 1; i < n; ++i) { ... }
      dnl (don't confuse -AA with -Aa which is similar but without the std::
      dnl namespace).
      dnl
      dnl Note that using -AA means that user code must be built with -AA
      dnl (which is the default on Itanium, but not on PARISC:
      dnl  http://h21007.www2.hp.com/dspp/tech/tech_TechDocumentDetailPage_IDX/1,1701,5520,00.html
      dnl )
      dnl Also -ext (which can also be spelled +e):
      dnl "Allow various C++ extensions.  Currently -ext enables the 64 bit
      dnl integer (long long) data type."  This seems to be implied by -AA
      dnl but otherwise is needed to allow "cout << (long long)42;".
      ANSI_CXXFLAGS="-AA"
      dnl +w turns on more warnings.
      dnl +W<n1>,<n2>,... suppresses warnings n1, n2, ...
      dnl 336 is "Entire translation unit was empty" (which dummy.cc is).
      WARNING_CXXFLAGS="+w +W336" ;;
    cxx|*/cxx)
      dnl Select ANSI mode ('-std strict_ansi' is needed for ANSI iostream
      dnl as '-std ansi' gives a pre-standard AT&T-compatible version).
      dnl If this proves to be a problem, try '-std ansi -D__USE_STD_IOSTREAM'
      ANSI_CXXFLAGS="-std strict_ansi" ;;
    CC|*/CC)
      dnl Select ANSI mode for SGI's CC
      ANSI_CXXFLAGS="-LANG:std" ;;
  esac

  dnl Check that the compiler recognises any flags we picked above to avoid
  dnl problems with compilers whose names clash with those above.
  dnl E.g. Solaris, FreeBSD, and MacOS X all have a "CC" which isn't SGI's.
  if test -n "$ANSI_CXXFLAGS" ; then
    AC_TRY_COMPILE([], [], , ANSI_CXXFLAGS=)
  fi

  if test -n "$ANSI_CXXFLAGS" ; then
    AC_MSG_RESULT([$ANSI_CXXFLAGS])
  else
    dnl Perhaps we should try to compile some code which uses iostream, for
    dnl scoping, etc, but maintaining such a snippet to cover everything we
    dnl require which any C++ compiler may not support is tricky.
    AC_MSG_RESULT([none known for $CXX])
  fi
fi

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

dnl Checks for perl (needed to regenerate some of the documentation)
AC_PATH_PROG(PERL, perl, [])
if test x$USE_MAINTAINER_MODE = xyes; then
  test -z "$PERL" && AC_MSG_ERROR([perl is required in maintainer mode])
fi

dnl Make DATADIR available to the library
dnl This isn't currently used.  If we reenable it, it needs to be renamed
dnl to avoid a clash with a mingw header.
dnl AC_DEFINE_DIR(DATADIR, datadir, [Location of platform independent support files])

dnl Checks for dot.  (Diagrams in the documentation)
AC_PATH_PROG(DOT, dot)
if test x$USE_MAINTAINER_MODE = xyes; then
  test -z "$DOT" && AC_MSG_ERROR([dot is required in maintainer mode])
fi
DOXYGEN_DOT_PATH=`echo "$DOT" | sed 's!/dot$!!'`
AC_SUBST(DOXYGEN_DOT_PATH)

dnl Check for latex. (Needed to make some more of the documentation)
AC_PATH_PROG(LATEX, latex, [])
if test x$USE_MAINTAINER_MODE = xyes; then
  test -z "$LATEX" && AC_MSG_ERROR([latex is required in maintainer mode])
fi

dnl Check for makeindex. (Needed to make some more of the documentation)
AC_PATH_PROG(MAKEINDEX, makeindex, [])
if test x$USE_MAINTAINER_MODE = xyes; then
  test -z "$MAKEINDEX" && AC_MSG_ERROR([makeindex is required in maintainer mode])
fi

dnl Check for doxygen. (Needed to make some more of the documentation)
AC_PATH_PROG(DOXYGEN, doxygen, [])
if test x$USE_MAINTAINER_MODE = xyes; then
  test -z "$DOXYGEN" && AC_MSG_ERROR([doxygen is required in maintainer mode])
fi

dnl Check for dvips. (Needed to make some more of the documentation)
AC_PATH_PROG(DVIPS, dvips, [])
if test x$USE_MAINTAINER_MODE = xyes; then
  test -z "$DVIPS" && AC_MSG_ERROR([dvips is required in maintainer mode])
fi

dnl Check for help2man. (Needed to make man pages from "--help" output).
AC_PATH_PROG(HELP2MAN, help2man, [])
if test x$USE_MAINTAINER_MODE = xyes; then
  test -z "$HELP2MAN" && AC_MSG_ERROR([help2man is required in maintainer mode])
fi

dnl Check whether we need -ldl for dlsym() etc.
dnl No longer used...
dnl AC_TRY_LINK_FUNC(dlsym, ,
dnl	[AC_CHECK_LIB(dl, dlsym, [DL_LIBS="-ldl"])])
dnl AC_SUBST(DL_LIBS)

dnl Checks for header files.
AC_CHECK_HEADERS(fcntl.h limits.h sys/errno.h, [], [], [ ])

dnl If valgrind is installed and new enough, we use it for leak checking in the
dnl testsuite.  If VALGRIND is set to an empty value, then skip the check and
dnl don't use valgrind.
if test -n "${VALGRIND-unset}" ; then
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

    dnl Check that the installed valgrind version works, and see if it needs
    dnl --tool=memcheck to work.
    if $VALGRIND -q true 2> /dev/null ; then
       dnl All is OK.
       :
    elif $VALGRIND --tool=memcheck -q true 2> /dev/null ; then
       dnl The valgrind version installed requires --tool=memcheck
       VALGRIND="$VALGRIND --tool=memcheck"
    else
       dnl The valgrind detected doesn't work!  Perhaps this is an x86_64 box
       dnl with a 32 bit valgrind.
       VALGRIND=
    fi
  fi
fi

if test -n "$VALGRIND" ; then
  AC_DEFINE(HAVE_VALGRIND, 1,
	    [Define if valgrind is installed and it supports VALGRIND_COUNT_LEAKS])
fi

dnl Checks for library functions.
AC_FUNC_MEMCMP
AC_CHECK_FUNCS(strerror)

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

dnl ***************************
dnl * Select modules to build *
dnl ***************************

dnl Find out what we've been asked to build

dnl muscat36 defaults to off
AC_ARG_ENABLE(backend_muscat36,
[  --enable-backend-muscat36 Enable (old) Muscat 3.6 databases (default=no)],
[case ${enableval} in
  yes) build_muscat36=true ;;
  no)  build_muscat36=false ;;
  *) AC_MSG_ERROR(bad value ${enableval} for --enable-backend-muscat36) ;;
esac],[build_muscat36=false])

AC_ARG_ENABLE(backend_quartz,
[  --enable-backend-quartz Enable the quartz database backend (default=yes)],
[case ${enableval} in
  yes) build_quartz=true ;;
  no)  build_quartz=false ;;
  *) AC_MSG_ERROR(bad value ${enableval} for --enable-backend-quartz) ;;
esac],[build_quartz=true])

AC_ARG_ENABLE(backend_flint,
[  --enable-backend-flint Enable the flint database backend (default=yes)],
[case ${enableval} in
  yes) build_flint=true ;;
  no)  build_flint=false ;;
  *) AC_MSG_ERROR(bad value ${enableval} for --enable-backend-flint) ;;
esac],[
  dnl We'd need to implement a flint locking strategy for MSDOS before we can
  dnl build flint there.
  case $host_os in
    *djgpp* | *msdos* ) build_flint=false ;;
    *) build_flint=true ;;
  esac
])

AC_ARG_ENABLE(backend_inmemory,
[  --enable-backend-inmemory Enable the inmemory database backend (default=yes)],
[case ${enableval} in
  yes) build_inmemory=true ;;
  no)  build_inmemory=false ;;
  *) AC_MSG_ERROR(bad value ${enableval} for --enable-backend-inmemory) ;;
esac],[build_inmemory=true])

AC_ARG_ENABLE(backend_remote,
[  --enable-backend-remote Enable the remote database backend (default=yes)],
[case ${enableval} in
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

      dnl check for socklen_t
      TYPE_SOCKLEN_T
    fi
  fi
  if test false != "$build_remote" ; then
    XAPIAN_LDFLAGS="$XAPIAN_LDFLAGS $REMOTE_LDFLAGS"
  fi
fi

AC_ARG_ENABLE(quiet,
[  --enable-quiet          Enable quiet building (default=no)],
[case ${enableval} in
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
AM_CONDITIONAL(BUILD_BACKEND_FLINT, test true = "$build_flint")
AM_CONDITIONAL(BUILD_BACKEND_INMEMORY, test true = "$build_inmemory")
AM_CONDITIONAL(BUILD_BACKEND_REMOTE, test true = "$build_remote")
dnl Set a flag for AM when shared libraries are enabled.
dnl AM_CONDITIONAL(ENABLE_SHARED, test yes = "$enable_shared")

dnl See if we have fdatasync, and what libraries are needed for it.
SAVE_LIBS="$LIBS"
LIBS=
AC_SEARCH_LIBS(fdatasync, rt, [XAPIAN_LDFLAGS="$LIBS $XAPIAN_LDFLAGS"])
LIBS="$SAVE_LIBS"

AC_CHECK_FUNCS(fsync)

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
    AC_CHECK_FUNC(pread,
       [AC_MSG_CHECKING([for any prototype needed for pread])
	AC_CACHE_VAL([xo_cv_pread_prototype],
	  [
	    for p in ' ' \
	      'extern "C" ssize_t pread(int, void *, size_t, off_t) throw ();' \
	      'extern "C" ssize_t pread(int, void *, size_t, off_t);' ; do
	      AC_TRY_COMPILE([
#include <sys/types.h>
#include <unistd.h>
$p
	      ],[
	        char b[256];
	        pread(1, b, 256, 20);
	      ],[
		xo_cv_pread_prototype="$p"
		break
	      ])
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
	    AC_DEFINE_UNQUOTED(PREAD_PROTOTYPE, [$xo_cv_pread_prototype],
			       [explicit prototype needed for pread (if any)])
	  fi
	])
    AC_CHECK_FUNC(pwrite,
       [AC_MSG_CHECKING([for any prototype needed for pwrite])
	AC_CACHE_VAL([xo_cv_pwrite_prototype],
	  [
	    for p in ' ' \
	      'extern "C" ssize_t pwrite(int, const void *, size_t, off_t) throw ();' \
	      'extern "C" ssize_t pwrite(int, const void *, size_t, off_t);' ; do
	      AC_TRY_COMPILE([
#include <sys/types.h>
#include <unistd.h>
$p
	      ],[
	        const char *p = "hello";
	        pwrite(1, p, 5, 20);
	      ],[
		xo_cv_pwrite_prototype="$p"
		break
	      ])
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
	    AC_DEFINE_UNQUOTED(PWRITE_PROTOTYPE, [$xo_cv_pwrite_prototype],
			       [explicit prototype needed for pwrite (if any)])
	  fi
	])
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
[case ${enableval} in
  full) use_debug_paranoid=yes ; use_debug=yes ; use_debug_verbose=yes ;;
  yes) use_debug_paranoid=yes ; use_debug=yes ;;
  partial) use_debug=yes ;;
  profile) use_debug_profile=yes ;;
  no) use_debug=no ;;
  *) AC_MSG_ERROR(bad value ${enableval} for --enable-debug) ;;
esac])

AC_ARG_ENABLE(debug-verbose,
[  --enable-debug-verbose  Turn on debugging messages (no|yes)(default=no)],
[case ${enableval} in
  yes) use_debug_verbose=yes ;;
  no)  use_debug_verbose=no ;;
  *) AC_MSG_ERROR(bad value ${enableval} for --enable-debug-verbose) ;;
esac])

dnl Now set appropriate defines

if test yes = "$use_debug"; then
AC_DEFINE(XAPIAN_DEBUG,,
  [Define if you want debugging to be enabled (will cause some slow down)])
fi

if test yes = "$use_debug_paranoid"; then
AC_DEFINE(XAPIAN_DEBUG_PARANOID,,
  [Define if you want paranoid debugging to be enabled (will cause
  significant slow-down)])
fi

if test yes = "$use_debug_verbose"; then
AC_DEFINE(XAPIAN_DEBUG_VERBOSE,,
  [Define if you want lots of debugging messages])
fi

if test yes = "$use_debug_profile"; then
AC_DEFINE(XAPIAN_DEBUG_PROFILE,,
  [Define if you want code profiling messages])
fi

dnl ******************************
dnl * Set special compiler flags *
dnl ******************************

dnl Set flags to control warnings (enable more, or disable annoying ones).
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
      AM_CXXFLAGS="$AM_CXXFLAGS -Wall -W -Wredundant-decls -Wundef -Wpointer-arith -Wcast-qual -Wcast-align -Wno-multichar -Wno-long-long -fno-gnu-keywords"
      dnl Automatically add -Werror if maintainer mode is enabled and we're
      dnl using GCC3 or newer.  We don't do this for older GCCs as GCC 2.95
      dnl issues spurious warnings.
      dnl Also only add -Wendif-labels in this case (it was added in GCC 3.0).
      if test x$USE_MAINTAINER_MODE = xyes; then
	dnl GCC2 reports e.g. "2.7.2.3" or "2.95.4".  3.0 reports e.g. "3.0.4"
	dnl but somewhere in the 3.X series the output became more verbose.
	dnl So for simplicitly, we test for the earlier versions.
	case `$CXX --version 2>&1` in
	[[12]].*) ;; dnl GCC < 3
	3.0*) AM_CXXFLAGS="$AM_CXXFLAGS -Wendif-labels -Werror" ;;
	dnl -Wshadow spews false positives with GCC 3.0.4
	*) AM_CXXFLAGS="$AM_CXXFLAGS -Wshadow -Wendif-labels -Werror" ;;
	esac
      fi
    ])
else
    dnl WARNING_CXXFLAGS is set above.
    AM_CXXFLAGS="$WARNING_CXXFLAGS"
fi

AH_BOTTOM(
[/* Disable stupid MSVC "performance" warning for converting int to bool. */
#ifdef _MSC_VER
# pragma warning(disable:4800)
#endif])

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

dnl Libtool sets this (to yes|no|unknown) and we use it in xapian-config.
AC_SUBST(link_all_deplibs_CXX)

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
 backends/flint/Makefile
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
 examples/Makefile
 getopt/Makefile
 queryparser/Makefile
 docs/doxygen_api_conf
 docs/doxygen_full_conf
 xapian.spec
 ])
AC_CONFIG_FILES([tests/runtest], [chmod +x tests/runtest])
AC_CONFIG_FILES([xapian-config], [chmod +x xapian-config])
AC_OUTPUT

MAJOR_VERSION=`echo "$VERSION"|sed 's/\..*//'`
[MINOR_VERSION=`echo "$VERSION"|sed 's/[^.]*\.//;s/\..*//'`]
[REVISION=`echo "$VERSION"|sed 's/.*\.//;s/_svn[0-9]*$//'`]
test -d include/xapian || mkdir include/xapian
dnl MAIN_VERSION is VERSION without any _svn6789 suffix.
MAIN_VERSION="$MAJOR_VERSION.$MINOR_VERSION.$REVISION"
cxxcpp_flags=-I.
for backend in MUSCAT36 QUARTZ FLINT INMEMORY REMOTE ; do
  if test -z "`eval echo \"\\$BUILD_BACKEND_${backend}_TRUE\"`" ; then
    cxxcpp_flags="$cxxcpp_flags -DXAPIAN_HAS_${backend}_BACKEND"
  fi
done
dnl Use @ around $MAIN_VERSION so we get " in the final output.
rm -f include/xapian/version.h.tmp
$CXXCPP $cxxcpp_flags -DSTRING_VERSION="\"@$MAIN_VERSION@\"" -DMAJOR_VERSION="\"$MAJOR_VERSION\"" -DMINOR_VERSION="\"$MINOR_VERSION\"" -DREVISION="\"$REVISION\"" $srcdir/include/xapian/version_h.cc|grep '"'|${SED-sed} 's/^ *//;s/ *$//;/^#/d;s/" *,//;s/"//g;s/@/"/g;s/  */ /g;s/,$//' > include/xapian/version.h.tmp
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
