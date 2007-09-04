dnl Process this file with autoconf to produce a configure script.

dnl Need autoconf 2.50 or later for many features
dnl 2.54 needed for automake 1.6
dnl 2.57 fixes the annoying warnings from configure on FreeBSD
dnl 2.58 is needed by automake 1.8.5
dnl 2.59 was released the same day as 2.58 to fix a problem
AC_PREREQ(2.59)
dnl NB When updating the version for a release, update LIBRARY_VERSION_INFO
dnl below too.
AC_INIT(xapian-core, 1.0.2)dnl FIXME: bugreport addr as third argument
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
dnl 12:0:1 0.9.6 Added Xapian::version_string(), etc
dnl 13:0:0 0.9.7 Rewritten Xapian::Error class
dnl 13:0:0 0.9.8 No API changes (failed to bump revision)
dnl 13:1:0 0.9.9 No API changes
dnl 13:2:0 0.9.10 No API changes
dnl 14:0:0 1.0.0 Numerous API changes
dnl 15:0:0 1.0.1 Error::error_string changed type
dnl 16:0:1 1.0.2 New QueryParser flags, etc
LIBRARY_VERSION_INFO=16:0:1
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
dnl 1.9 reduces Makefile.in size, and 1.9.5 was in Debian sarge
dnl But the RPM spec file runs autoreconf, and SLES 9 has 1.8.3.
AM_INIT_AUTOMAKE([1.8.3 -Wportability])
AC_CONFIG_SRCDIR(matcher/andpostlist.cc)

AC_CONFIG_HEADERS(config.h)

dnl We don't use Fortran, but libtool 1.5 insists on probing for f77 which just
dnl makes configure take longer to run.  We force it not to using the method
dnl suggested here (which also reduces configure size by 12%):
dnl http://article.gmane.org/gmane.comp.gnu.libtool.general/8172
m4_defun([_LT_AC_LANG_F77_CONFIG], [:])
dnl Eliminating the unused code for gcj and rc probes reduces the size of the
dnl generated configure script by a further 15%!
m4_defun([_LT_AC_LANG_GCJ_CONFIG], [:])
m4_defun([_LT_AC_LANG_RC_CONFIG], [:])

dnl Use libtool to manage our libraries
AC_PROG_LIBTOOL

AM_CXXFLAGS=

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
dnl to either install one or set CXX if there's one configure failed to find.
dnl It's probably worthwhile checking that the C++ compiler actually works
dnl anyway!
if test -n "$CXX" ; then
  AC_LANG_CPLUSPLUS
  AC_MSG_CHECKING([whether $CXX is a working C++ compiler])
  AC_CACHE_VAL([xo_cv_cxx_works],
    [
    AC_TRY_RUN([int main() {}],
	       xo_cv_cxx_works=yes,
	       xo_cv_cxx_works=no,
	       AC_TRY_LINK([], [], xo_cv_cxx_works=yes, xo_cv_cxx_works=no))
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

AC_DEFUN([XAPIAN_TEST_LINKER_FLAG],
  [
  AC_MSG_CHECKING([for $CXX -Wl,$1])
  AC_CACHE_VAL([xo_cv_$2],
    [
    flag=$1
    if $CXX -Wl,$1 2>&1 >/dev/null|grep -e $1 >/dev/null 2>&1; then
      dnl The error message contains the flag name - it must be a
      dnl complaint that the option is unrecognized (doing it this
      dnl way allows it to work regardless of the i18n in use):
      dnl ld: unrecognized option '--enable-runtime-pseudo-reloc'
      xo_cv_$2=no
      $4
    else
      xo_cv_$2=yes
      $3
    fi
    ])
  AC_MSG_RESULT([$xo_cv_$2])
  ])

ldflags=
if test yesyes = "$GXX$enable_shared" ; then
  case $host_os in
    *mingw* | *cygwin*)
      XAPIAN_TEST_LINKER_FLAG([--enable-runtime-pseudo-reloc], [enable_runtime_pseudo_reloc],
	[ldflags="-Wl,$flag"],
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
AC_SUBST(ldflags)

WARNING_CXXFLAGS=
AC_MSG_CHECKING([for $CXX options to enable ANSI C++ mode])
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
      dnl +wlint turns on "lint-like" warnings.
      dnl +W<n1>,<n2>,... suppresses warnings n1, n2, ...
      dnl 2340 (remark) "value copied to temporary, reference to temporary
      dnl	use", in: throw Xapian::UnimplementedError("...");
      dnl 2401 "destructor for base class ... is non-virtual" (we don't need a
      dnl	virtual destructor for RefCntBase, since we never delete its
      dnl	subclasses by a RefCntBase *).
      dnl 3348 "declaration hides constant ..." which seems to misfire!
      dnl 4255 (remark) "padding size of struct "..." with ... bytes to
      dnl	alignment boundary".
      dnl 4273 "floating-point equality and inequality comparisons may be
      dnl	inappropriate due to roundoff common in floating-point computation"
      dnl	No obvious workaround for when you really do want == or !=.
      dnl 4285 "operator= does not have a check for the sourc and destination
      dnl	addresses being non-identical" - fires for AutoPtr which
      dnl	includes such a check indirectly (internaltest's autoptr1 check this).
      dnl 20201 "Memory leak is detected" which triggers for "return new Foo;"!
      WARNING_CXXFLAGS="+w +wlint +W2340,2401,3348,4255,4273,4285,20201" ;;
    cxx|*/cxx)
      dnl Select ANSI mode ('-std strict_ansi' is needed for ANSI iostream
      dnl as '-std ansi' gives a pre-standard AT&T-compatible version).
      dnl If this proves to be a problem, try '-std ansi -D__USE_STD_IOSTREAM'
      ANSI_CXXFLAGS="-std strict_ansi"
      dnl Check that the compiler recognises these flags to avoid problems
      dnl with other compilers named cxx.
      AC_TRY_COMPILE([], [], , ANSI_CXXFLAGS=) ;;
    CC|*/CC)
      dnl Could be Sun's, SGI's, or something else entirely.
      case `$CXX -V 2>&1` in
      *Sun\ C++*)
	dnl Select ANSI conforming STL - for more information see:
	dnl http://developers.sun.com/prodtech/cc/articles/cmp_stlport_libCstd.html
	dnl and:
	dnl http://developers.sun.com/prodtech/cc/documentation/ss9_docs/mr/READMEs/c++_faq.html#bb13
	dnl (Looks like -features=tmplife was added around 2000 so just pass
	dnl it unconditionally)
	ANSI_CXXFLAGS="-library=stlport4 -features=tmplife" ;;
      *)
	case `$CXX -v 2>&1` in
	MIPSpro*)
	  dnl On stderr: MIPSpro Compilers: Version 7.4.2m
	  dnl Select ANSI mode for SGI's CC with "-LANG:std".  Also, we have to
	  dnl specify "-ptused" or we get strange template linking errors.
	  ANSI_CXXFLAGS="-LANG:std -ptused" ;;
	esac
	;;
      esac
      ;;
  esac

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

dnl Some versions of Sun's C++ compiler reportedly need an explicit -lm.
dnl The maths functions we use include: exp log ceil fabs sqrt
AC_MSG_CHECKING([if -lm is required for maths functions])
dnl Don't use a constant argument to log as the compiler might simply evaluate
dnl the whole expression at compile time.
AC_TRY_LINK([#include <math.h>
#include <stdlib.h>], [double a = log((double)rand());], [AC_MSG_RESULT(no)], [
    LIBS="-lm $LIBS"
    AC_TRY_LINK([#include <math.h>
#include <stdlib.h>], [double a = log((double)rand());],
	[AC_MSG_RESULT(yes)],
	[AC_MSG_ERROR([Failed to link a C++ program using log()])
    ])
])

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
AC_CHECK_TYPE(ssize_t, int)

AC_TYPE_PID_T

AC_TYPE_MODE_T

dnl Check for a stringstream implementation.  (If not present, we roll
dnl one ourselves).
AC_CHECK_HEADERS(sstream, [], [], [ ])
dnl Check for a new streams implementation.
AC_CHECK_HEADERS(streambuf, [], [], [ ])

dnl Check for perl (needed to generate some sources and documentation).
AC_PATH_PROG(PERL, perl, [])
if test x$USE_MAINTAINER_MODE = xyes; then
  test -z "$PERL" && AC_MSG_ERROR([perl is required in maintainer mode])
fi

dnl We only need to set docdir for compatibility with autoconf < 2.60 - this
dnl code can be removed once we move to requiring autoconf 2.60 or newer.
test -n "$docdir" || docdir='${datadir}/doc/${PACKAGE_TARNAME}'
AC_SUBST(docdir)

AC_ARG_ENABLE(documentation,
  [AS_HELP_STRING([--enable-documentation], [enable make rules to rebuild documentation [default=maintainer-mode]])],
  [case ${enableval} in
    yes|no) ;;
    *) AC_MSG_ERROR([bad value ${enableval} for --enable-documentation]) ;;
  esac])
test -z "$enable_documentation" && enable_documentation=$USE_MAINTAINER_MODE
AM_CONDITIONAL(DOCUMENTATION_RULES, test x"$enable_documentation" = xyes)
AM_CONDITIONAL(MAINTAINER_NO_DOCS, test x"$USE_MAINTAINER_MODE$enable_documentation" = xyesno)

dnl Make DATADIR available to the library
dnl This isn't currently used.  If we reenable it, it needs to be renamed
dnl to avoid a clash with a mingw header.
dnl AC_DEFINE_DIR(DATADIR, datadir, [Location of platform independent support files])

if test x"$enable_documentation" = xyes ; then
  dnl Checks for dot.  (Diagrams in the documentation)
  AC_PATH_PROG(DOT, dot)
  test -z "$DOT" && AC_MSG_ERROR([dot (part of the graphviz package) is required to build documentation])
  DOXYGEN_DOT_PATH=`echo "$DOT" | sed 's!/dot$!!'`
  AC_SUBST(DOXYGEN_DOT_PATH)

  dnl FIXME: pdflatex is needed to make some more of the documentation but
  dnl doxygen expects it to be on PATH.

  dnl Check for makeindex. (Needed to make some more of the documentation)
  AC_PATH_PROG(MAKEINDEX, makeindex, [])
  test -z "$MAKEINDEX" && AC_MSG_ERROR([makeindex is required to build documentation])

  dnl Check for doxygen. (Needed to make some more of the documentation)
  AC_PATH_PROG(DOXYGEN, doxygen, [])
  test -z "$DOXYGEN" && AC_MSG_ERROR([doxygen is required to build documentation])

  dnl Check for help2man. (Needed to make man pages from "--help" output).
  AC_PATH_PROG(HELP2MAN, help2man, [])
  test -z "$HELP2MAN" && AC_MSG_ERROR([help2man is required to build documentation])

  dnl Check for rst2html. (Needed to make HTML from reStructuredText format)
  AC_PATH_PROG(RST2HTML, rst2html, [])
  test -z "$RST2HTML" && AC_MSG_ERROR([rst2html is required to build documentation])
fi

dnl Check whether we need -ldl for dlsym() etc.
dnl No longer used...
dnl AC_TRY_LINK_FUNC(dlsym, ,
dnl	[AC_CHECK_LIB(dl, dlsym, [DL_LIBS="-ldl"])])
dnl AC_SUBST(DL_LIBS)

dnl Checks for header files.
AC_CHECK_HEADERS([fcntl.h limits.h sys/errno.h sys/select.h], [], [], [ ])

dnl If valgrind is installed and new enough, we use it for leak checking in the
dnl testsuite.  If VALGRIND is set to an empty value, then skip the check and
dnl don't use valgrind.
dnl
dnl Default to what modern valgrind knows, so the user can specify valgrind
dnl when running runtest even if it doesn't get picked up here.
VALGRIND_LOG_FILE_OPT=--log-file
if test -n "${VALGRIND-unset}" ; then
  AC_PATH_PROG(VALGRIND, valgrind, [])
  if test -n "$VALGRIND" ; then
    dnl Check that the valgrind version installed supports the most recently
    dnl added client request which we use - VALGRIND_DO_LEAK_CHECK is ages old,
    dnl VALGRIND_COUNT_ERRORS was added at the same time as VALGRIND_COUNT_LEAKS
    AC_MSG_CHECKING([for VALGRIND_COUNT_LEAKS])
    AC_EGREP_CPP(yes,
      [#include <valgrind/memcheck.h>
       #ifdef VALGRIND_COUNT_LEAKS
       yes
       #endif
      ], AC_MSG_RESULT(found),
      AC_MSG_RESULT(not supported)
      [VALGRIND=])
  fi

  if test -n "$VALGRIND" ; then
    dnl Check that the installed valgrind version works, and see if it needs
    dnl --tool=memcheck to work (older and newer versions don't, but some in
    dnl between do).  Valgrind 2.1.2 and later have a --log-file option but
    dnl 2.1.1 and older had --logfile instead, so we need to check this too.
    AC_MSG_CHECKING([for valgrind options to use])
    if $VALGRIND --log-file=config.valgrind.tmp -q true 2> /dev/null ; then
      dnl Test the case which matches a modern valgrind first so we handle
      dnl the common case quickly with a single test.
      AC_MSG_RESULT([--log-file])
    else
      dnl Now we have to test individual options.
      if $VALGRIND --tool=memcheck -q true 2> /dev/null ; then
	dnl This valgrind version requires --tool=memcheck
	VALGRIND="$VALGRIND --tool=memcheck"
	AC_MSG_RESULT([--tool=memcheck])
	AC_MSG_CHECKING([for valgrind options to use])
      fi
      if $VALGRIND --log-file=config.valgrind.tmp -q true 2>/dev/null ; then
	AC_MSG_RESULT([--log-file])
      elif $VALGRIND --logfile=config.valgrind.tmp -q true 2>/dev/null ; then
	VALGRIND_LOG_FILE_OPT=--logfile
	AC_MSG_RESULT([--logfile])
      else
	dnl The valgrind detected doesn't seem to work!  Perhaps this is an
	dnl x86_64 box with a 32 bit valgrind.
	AC_MSG_RESULT([$VALGRIND doesn't work])
	VALGRIND=
      fi
    fi
    rm -f config.valgrind.tmp*
  fi
fi
AC_SUBST(VALGRIND_LOG_FILE_OPT)

if test -n "$VALGRIND" ; then
  AC_DEFINE(HAVE_VALGRIND, 1,
	    [Define if valgrind is installed and supports VALGRIND_COUNT_LEAKS])
fi

dnl Checks for library functions.
AC_FUNC_MEMCMP
AC_CHECK_FUNCS(strerror hstrerror)

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

dnl ***************************
dnl * Select modules to build *
dnl ***************************

dnl Check which database backends should be built.

AC_DEFUN([XAPIAN_BACKEND_ENABLE],
  [AC_ARG_ENABLE(backend_$1,
    [AS_HELP_STRING([--enable-backend-$1], [build the $1 database backend [default=yes]])],
    [case $enableval in
      yes|no) ;;
      *) AC_MSG_ERROR([Invalid option: '--enable-backend-$1=$enableval']) ;;
    esac], [enable_backend_$1=yes])
  ])

dnl When adding a new backend, update INSTALL too.
XAPIAN_BACKEND_ENABLE(flint)
XAPIAN_BACKEND_ENABLE(inmemory)
XAPIAN_BACKEND_ENABLE(quartz)
XAPIAN_BACKEND_ENABLE(remote)

if test yes = "$enable_backend_flint" ; then
  dnl We would need to implement a flint locking strategy for MSDOS before we
  dnl could build flint there.
  case $host_os in
    *djgpp* | *msdos* ) enable_backend_flint=no ;;
  esac
fi

if test yes = "$enable_backend_flint" ; then
  dnl We use zlib for compressing tags in flint.  We could automatically
  dnl disable support for flint, but overall that probably does more harm
  dnl than good - it's most likely that someone just forgot to install the
  dnl -dev package for zlib and flint is the default backend we want people
  dnl to use. 

  dnl Check for zlib.h.
  AC_CHECK_HEADERS(zlib.h, [], [
    AC_MSG_ERROR([zlib.h not found - required for flint (you may need to install the zlib1g-dev or zlib-devel package)])
    ], [ ])

  dnl Check for zlibVersion in -lz.
  SAVE_LIBS=$LIBS
  LIBS=
  dnl mingw build needs -lzlib or -lzdll.
  AC_SEARCH_LIBS([zlibVersion], [z zlib zdll], [], [
    AC_MSG_ERROR([zlibVersion() not found in -lz - required for flint (you may need to install the zlib1g-dev or zlib-devel package)])
    ])
  if test x != x"$LIBS" ; then
    XAPIAN_LDFLAGS="$XAPIAN_LDFLAGS $LIBS"
  fi
  LIBS=$SAVE_LIBS
fi

REMOTE_LDFLAGS=
if test "$enable_backend_remote" = yes ; then
  AC_PREPROC_IFELSE([
#ifdef __WIN32__
#error WIN32
#endif
  ], [win32=no], [win32=yes])

  case $host_os-$win32 in
    *-yes )
      dnl For mingw and msvc we have an alternative implementation which
      dnl doesn't need fork().  Also the "prog" variant isn't built
      dnl currently so socketpair() isn't required either.
      dnl
      dnl We need -lws2_32 for gethostbyname(), etc.  Hardcode that knowledge
      dnl here because AC_SEARCH_LIBS fails to link the test program (I think
      dnl because its prototype has the wrong calling convention).
      REMOTE_LDFLAGS=-lws2_32
      ;;
    *djgpp* | *msdos* )
      dnl DJGPP has a dummy implementation of fork which always fails.
      enable_backend_remote=no ;;
    *)
      dnl On Unix, we need fork and socketpair for the remotebackend.
      AC_CHECK_FUNCS(fork)
      SAVE_LIBS=$LIBS
      LIBS=
      if test "$ac_cv_func_fork" = no ; then
	enable_backend_remote=no
      else
	dnl Check if -lsocket is required for socketpair (Solaris needs it).
	AC_SEARCH_LIBS(socketpair, socket, [], [enable_backend_remote=no])
	if test "$enable_backend_remote" = yes ; then
	  AC_DEFINE(HAVE_SOCKETPAIR, 1,
		    [Define to 1 if you have the 'socketpair' function.])
	  dnl Check if -lnsl is required for gethostbyname (Solaris needs it).
	  AC_SEARCH_LIBS(gethostbyname, nsl, [], [enable_backend_remote=no])
	  if test "$enable_backend_remote" = yes ; then
	    dnl Check if -lnsl is required for gethostbyname (Solaris needs it).
	    AC_SEARCH_LIBS(gethostbyaddr, nsl, [], [enable_backend_remote=no])
	    if test "$enable_backend_remote" = yes ; then
	      REMOTE_LDFLAGS=$LIBS
	    fi
	  fi
	fi
      fi
      LIBS="$SAVE_LIBS"
      ;;
  esac

  if test "$enable_backend_remote" = yes ; then
    TYPE_SOCKLEN_T
    XAPIAN_LDFLAGS="$XAPIAN_LDFLAGS $REMOTE_LDFLAGS"
  fi
fi

AC_ARG_ENABLE(quiet,
  [AS_HELP_STRING([--enable-quiet], [enable quiet building [default=no]])],
  [case ${enableval} in
    yes|no) ;;
    *) AC_MSG_ERROR([bad value ${enableval} for --enable-quiet]) ;;
  esac])

AC_ARG_ENABLE(visibility,
  [AS_HELP_STRING([--disable-visibility], [Disable use of GCC visibility [default=autodetect]])],
  [case ${enableval} in
    yes|no) ;;
    *) AC_MSG_ERROR([bad value ${enableval} for --disable-visibility]) ;;
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

vpath_build=no
if test "`pwd`" != "`cd $srcdir;pwd`" ; then
  vpath_build=yes
fi
AM_CONDITIONAL(VPATH_BUILD, test yes = "$vpath_build")

dnl Turn off compilation of anything that we don't have the requirements for

dnl Set conditionals to specify what we compile

AM_CONDITIONAL(BUILD_BACKEND_QUARTZ, test yes = "$enable_backend_quartz")
AM_CONDITIONAL(BUILD_BACKEND_FLINT, test yes = "$enable_backend_flint")
AM_CONDITIONAL(BUILD_BACKEND_INMEMORY, test yes = "$enable_backend_inmemory")
AM_CONDITIONAL(BUILD_BACKEND_REMOTE, test yes = "$enable_backend_remote")
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

dnl Which assertion types to enable in the code.

AC_ARG_ENABLE(assertions,
  [AS_HELP_STRING([--enable-assertions], [enable debug assertions (no|partial|yes) [default=no]])],
  [case $enableval in
    yes|partial|no) ;;
    *)
      AC_MSG_ERROR([Invalid option: '--enable-assertions=$enableval']) ;;
  esac])

AC_ARG_ENABLE(log,
  [AS_HELP_STRING([--enable-log], [generate a log of methods called, etc [default=no]])],
  [case $enableval in
    yes|profiling|no) ;;
    *)
      AC_MSG_ERROR([Invalid option: '--enable-log=$enableval']) ;;
  esac])

dnl Provide errors for old options which direct the user to the new ones.

AC_ARG_ENABLE(debug,
  [AS_HELP_STRING([--enable-debug], [Deprecated (use --enable-assertions and/or --enable-log instead)])],
  [case $enableval in
    full)
      AC_MSG_ERROR(['--enable-debug=full' is no longer supported, use '--enable-debug --enable-log' instead.]) ;;
    profile)
      AC_MSG_ERROR(['--enable-debug=profile' is no longer supported, use '--enable-log=profile' instead.]) ;;
    yes)
      AC_MSG_ERROR(['--enable-debug' is no longer supported, use '--enable-assertions' instead.]) ;;
    partial)
      AC_MSG_ERROR(['--enable-debug=partial' is no longer supported, use '--enable-assertions=partial' instead.]) ;;
    no)
      AC_MSG_ERROR(['--disable-debug' is no longer supported, use '--disable-assertions' instead.]) ;;
    *)
      AC_MSG_ERROR([Invalid option: '--enable-debug=$enableval']) ;;
  esac])

AC_ARG_ENABLE(debug-verbose,
  [AS_HELP_STRING([--enable-debug-verbose], [Deprecated (use --enable-log instead)])],
  [AC_MSG_ERROR(['--enable-debug-verbose' is no longer supported, use '--enable-log' instead.])])

dnl Set defines according to the --enable-assertions and --enable-log options
dnl given.

if test yes = "$enable_assertions" || test partial = "$enable_assertions" ; then
  AC_DEFINE(XAPIAN_DEBUG,,
    [Define if you want assertions (causes some slow-down)])
fi

if test yes = "$enable_assertions"; then
  AC_DEFINE(XAPIAN_DEBUG_PARANOID,,
    [Define if you want paranoid assertions (causes significant slow-down)])
fi

if test yes = "$enable_log"; then
  AC_DEFINE(XAPIAN_DEBUG_VERBOSE,,
    [Define if you want a log of methods called and other debug messages])
fi

if test profile = "$enable_log"; then
  AC_DEFINE(XAPIAN_DEBUG_PROFILE,,
    [Define if you want code profiling via the logging infrastructure])
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
      AM_CXXFLAGS="$AM_CXXFLAGS -Wall -W -Wredundant-decls -Wpointer-arith -Wcast-qual -Wcast-align -Wno-multichar -Wno-long-long -fno-gnu-keywords"

      dnl GCC2 reports e.g. "2.7.2.3" or "2.95.4".  3.0 reports e.g. "3.0.4"
      dnl but somewhere in the 3.X series the output became more verbose
      dnl (3.2 has the new style output - not sure about 3.1).  So we need to
      dnl run the output through head and sed to extract the version number.
      gxx_version=`$CXX --version 2>&1|head -1|${SED-sed} 's/^[[^ ]]* (GCC) //;s/ .*//'`

      case $gxx_version in
      2.96) dnl Oddly Redhat's "2.96" doesn't support -Wundef, though real
	    dnl GCC versions before and after it do!
	;;
      [[12]].*) dnl GCC < 3
	AM_CXXFLAGS="$AM_CXXFLAGS -Wundef" ;;
      3.0*) dnl -Wshadow spews false positives with GCC 3.0.4
	AM_CXXFLAGS="$AM_CXXFLAGS -Wundef" ;;
      3.*)
	AM_CXXFLAGS="$AM_CXXFLAGS -Wundef -Wshadow" ;;
      *)
	if test no = "$enable_visibility"; then
	  AM_CXXFLAGS="$AM_CXXFLAGS -Wundef -Wshadow"
	  AC_DEFINE(XAPIAN_DISABLE_VISIBILITY, 1, [Define to disable use of visibility attributes])
	else
	  dnl Turn on visibility support for GCC >= 4.0.
	  AM_CXXFLAGS="$AM_CXXFLAGS -Wundef -Wshadow -fvisibility=hidden"
	fi
	;;
      esac

      dnl Automatically add -Werror if maintainer mode is enabled and we're
      dnl using GCC4 or newer.  We don't do this for older GCCs as GCC 2.95
      dnl and some GCC 3.x compilers issue spurious warnings.
      if test x$USE_MAINTAINER_MODE = xyes; then
	case $gxx_version in
	[[123]].*) ;; dnl GCC < 4
	*) AM_CXXFLAGS="$AM_CXXFLAGS -Werror" ;;
	esac
      fi

      XAPIAN_TEST_LINKER_FLAG([-Bsymbolic-functions], [symbolic_functions],
	[XAPIAN_LDFLAGS="$XAPIAN_LDFLAGS $flag"])
    ])
else
  dnl WARNING_CXXFLAGS is set above.
  AM_CXXFLAGS="$WARNING_CXXFLAGS"
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

MAJOR_VERSION=`echo "$VERSION"|sed 's/\..*//'`
[MINOR_VERSION=`echo "$VERSION"|sed 's/[^.]*\.//;s/\..*//'`]
[REVISION=`echo "$VERSION"|sed 's/.*\.//;s/_svn[0-9]*$//'`]

dnl **************************
dnl * Build the output files *
dnl **************************

AC_CONFIG_FILES([
 Makefile
 tests/Makefile
 docs/Makefile
 docs/doxygen_api_conf
 docs/doxygen_full_conf
 xapian-core.spec
 ])
AC_CONFIG_FILES([tests/runtest], [chmod +x tests/runtest])
AC_CONFIG_FILES([tests/runsrv], [chmod +x tests/runsrv])
AC_CONFIG_FILES([xapian-config], [chmod +x xapian-config])
AC_CONFIG_FILES([makemanpage], [chmod +x makemanpage])
AC_CONFIG_FILES([docs/gen_codestructure_doc], [chmod +x docs/gen_codestructure_doc])
AC_CONFIG_FILES([generate-exceptions], [chmod +x generate-exceptions])
AC_CONFIG_FILES([languages/generate-allsnowballheaders], [chmod +x languages/generate-allsnowballheaders])
AC_OUTPUT

dnl There are no files generated by AC_OUTPUT in the following directories
dnl and we need to ensure they exist so that the rest of configure or make
dnl won't fail because they don't exist when srcdir != builddir.
if test yes = "$vpath_build" ; then
  for dir in include include/xapian languages queryparser ; do
    test -d "$dir" || mkdir "$dir"
  done
fi

dnl Generate include/xapian/version.h:

dnl MAIN_VERSION is VERSION without any _svn6789 suffix.
MAIN_VERSION="$MAJOR_VERSION.$MINOR_VERSION.$REVISION"
cxxcpp_flags=-I.
for backend in QUARTZ FLINT INMEMORY REMOTE ; do
  val=`eval echo "\\\$BUILD_BACKEND_${backend}_TRUE"`
  if test -z "$val" ; then
    cxxcpp_flags="$cxxcpp_flags -DXAPIAN_HAS_${backend}_BACKEND"
  fi
done
if test yes = "$GXX" ; then
    dnl If _GLIBCXX_DEBUG is being deliberately set or unset, do the same
    dnl when generating version.h.
    for flag in $CXXFLAGS $CPPFLAGS ; do
      case $flag in
      -D_GLIBCXX_DEBUG*|-U_GLIBCXX_DEBUG*) cxxcpp_flags="$cxxcpp_flags $flag" ;;
      esac
    done
fi
rm -f include/xapian/version.h.tmp
dnl "\r" in sed is a GNUism, but we only need to remove it on MS Windows
dnl where we'll always have GNU sed, and other sed's will just interpret
dnl it as "r" (Solaris sed) or maybe literal "\r" which won't match.
dnl
dnl Use @@ around $MAIN_VERSION so we get " in the final output.
$CXXCPP $cxxcpp_flags -DSTRING_VERSION="\"@@$MAIN_VERSION@@\"" -DMAJOR_VERSION="\"$MAJOR_VERSION\"" -DMINOR_VERSION="\"$MINOR_VERSION\"" -DREVISION="\"$REVISION\"" $srcdir/include/xapian/version_h.cc|${SED-sed} '/"/!d;s/^ *//;/^#/d;s/ *$//;s/" *,//;s/"//g;s/@@/"/g;s/  */ /g;s/ *,\r$//;s/ *,$//' > include/xapian/version.h.tmp
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
