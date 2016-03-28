dnl Process this file with autoconf to produce a configure script.

dnl NB When updating the version for a release, update LIBRARY_VERSION_INFO
dnl below too.
AC_INIT([xapian-core], [1.2.23], [https://xapian.org/bugs])
dnl See HACKING document for details of the reasons for required versions.
AC_PREREQ([2.64])
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
dnl LIBRARY_VERSION_INFO:
dnl 21:11:6 1.0.20 No API changes
dnl 22:0:0 1.2.0 Many API changes!
dnl 23:0:1 1.2.1 QueryParser::add_boolean_prefix()'s exclusive parameter.
dnl 23:1:1 1.2.2 No API changes
dnl 23:2:1 1.2.3 No API changes
dnl 24:0:2 1.2.4 Compactor
dnl 25:0:3 1.2.5 New min_wt overload for Enquire::get_eset()
dnl 26:0:4 1.2.6 QueryParser::set_max_wildcard_expansion() added
dnl 26:1:4 1.2.7 No API changes
dnl 26:2:4 1.2.8 No API changes
dnl 26:3:4 1.2.9 No API changes
dnl 26:4:4 1.2.10 No API changes
dnl 26:0:5 1.2.11 TermGenerator::set_stemming_strategy() added
dnl 27:0:5 1.2.12 Corrects error in 1.2.11's LIBRARY_VERSION_INFO
dnl 28:0:6 1.2.13 TermGenerator::set_max_word_length() added
dnl 28:1:6 1.2.14 No API changes
dnl 28:2:6 1.2.15 No API changes
dnl 28:3:6 1.2.16 No API changes
dnl 28:4:6 1.2.17 No API changes
dnl 28:5:6 1.2.18 No API changes
dnl 28:6:6 1.2.19 No API changes
dnl 28:7:6 1.2.20 No API changes
dnl 28:8:6 1.2.21 No API changes
dnl 29:0:7 1.2.22 FLAG_CJK_NGRAM added
dnl 29:1:7 1.2.23 No API changes (except new inline methods)
LIBRARY_VERSION_INFO=29:1:7
AC_SUBST([LIBRARY_VERSION_INFO])

LIBRARY_VERSION_SUFFIX=
AC_SUBST([LIBRARY_VERSION_SUFFIX])

dnl Disabled for stable release series.
dnl Default to "-1.1"; for no suffix, specify: --program-suffix=
dnl test x"$program_suffix" != xNONE || program_suffix=-1.1

dnl Where xapian.h, etc go.  In development release append "/xapian-1.1".
incdir=$includedir
AC_SUBST([incdir])

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

dnl See HACKING document for details of the reasons for required versions.
AM_INIT_AUTOMAKE([1.11 -Wportability tar-ustar no-dist-gzip dist-xz std-options])
AC_CONFIG_SRCDIR([matcher/multiandpostlist.cc])

AC_CONFIG_HEADERS([config.h])

AC_CONFIG_MACRO_DIR([m4])

dnl Use libtool to manage our libraries.
LT_PREREQ([2.2.8])
dnl Libtool doesn't know about -library=stdcxx4 yet, and also it checks for
dnl -library=stlport4 before we enable it, so trick it into not wanting to add
dnl -library=Cstd.
solaris_use_stlport4=yes
LT_INIT

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

dnl We don't use a C compiler to compile Xapian's code, but on some platforms
dnl (e.g. mingw) libtool uses $LTCC which defaults to $CC, and it's also use to
dnl build auxiliary tools like snowball and lemon.
AC_PROG_CC

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
AC_LANG_CPLUSPLUS

dnl Enable large file support if possible.
AC_SYS_LARGEFILE
dnl With xlC on AIX, -D_LARGE_FILES changes the ABI of std::string, so it
dnl also needs to be used when compiling user code.
abi_affecting_cxxflags=
if $GREP '^#define _LARGE_FILES 1$' confdefs.h > /dev/null 2>&1 ; then
  abi_affecting_cxxflags=-D_LARGE_FILES
fi
AC_SUBST([abi_affecting_cxxflags])

XAPIAN_LDFLAGS=
XAPIAN_LIBS=
AC_SUBST([XAPIAN_LDFLAGS])
AC_SUBST([XAPIAN_LIBS])

AC_DEFUN([XAPIAN_TEST_LINKER_FLAG],
  [
  AC_MSG_CHECKING([for $CXX -Wl,$1])
  AC_CACHE_VAL([xo_cv_$2],
    [
    flag=$1
    dnl The option in $1 will start with a dash so needs protecting.  We can't
    dnl use -e as Solaris /usr/bin/grep doesn't support that, so prepend an
    dnl empty group instead.
    if $CXX -Wl,$1 >&AS_MESSAGE_LOG_FD 2>&AS_MESSAGE_LOG_FD|grep '\(\)'$1 >/dev/null 2>&1; then
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

dnl Test if the compiler works with $1 added to CXXFLAGS; if it does, add $1 to
dnl variable $2.  If the test passes, also do $3; if it fails, also do $4.
AC_DEFUN([XAPIAN_TEST_CXXFLAGS],
  [
  XTC_save_CXXFLAGS=$CXXFLAGS
  CXXFLAGS="$CXXFLAGS $1"
  AC_TRY_COMPILE([], [],
		 [$2="${$2} $1"
		  CXXFLAGS=$XTC_save_CXXFLAGS
		  $3],
		 [CXXFLAGS=$XTC_save_CXXFLAGS
		  $4])
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
AC_SUBST([ldflags])

WARNING_CXXFLAGS=
ISO_CXXFLAGS=
AC_MSG_CHECKING([for $CXX options to enable ISO C++ mode])
if test yes = "$GXX"; then
  compiler=gcc
  AC_MSG_RESULT([none required])
else
  compiler=unknown
  dnl Some C++ compilers need a special switch to select ISO C++ mode.  Do
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
      ISO_CXXFLAGS="-AA"
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
      dnl 4285 "operator= does not have a check for the source and destination
      dnl	addresses being non-identical" - fires for AutoPtr which
      dnl	includes such a check indirectly (internaltest's autoptr1 check this).
      dnl 20201 "Memory leak is detected" which triggers for "return new Foo;"!
      WARNING_CXXFLAGS="+w +wlint +W2340,2401,3348,4255,4273,4285,20201" ;;
    cxx|*/cxx)
      dnl Select ANSI mode ('-std strict_ansi' is needed for ANSI iostream
      dnl as '-std ansi' gives a pre-standard AT&T-compatible version).
      dnl If this proves to be a problem, we could instead try
      dnl '-std ansi -D__USE_STD_IOSTREAM'.
      dnl
      dnl Check that the compiler recognises these flags to avoid problems
      dnl with other compilers named cxx.
      XAPIAN_TEST_CXXFLAGS([-std strict_ansi], [ISO_CXXFLAGS]) ;;
    CC|*/CC)
      dnl Could be Sun's, SGI's, or something else entirely.
      case `$CXX -V 2>&1` in
      *Sun\ C++*)
	compiler=sun
	dnl Select ANSI conforming STL - for more information see:
	dnl http://www.oracle.com/technetwork/server-storage/solarisstudio/documentation/cplusplus-faq-355066.html#LibComp5
	dnl -library=stdcxx4 is better, but needs "Solaris 10 update 10 or
	dnl Solaris 11".
	dnl
	dnl If the compiler supports -library=stdcxx4 but the user has
	dnl specified CXXFLAGS=-library=stlport4, then this should put
	dnl -library=stlport4 in ISO_CXXFLAGS, since the compiler will give
	dnl an error when passed -library=stlport4 -library=stdcxx4.
	for l in stdcxx4 stlport4 Cstd ; do
	  dnl We avoid -library-$l,Crun as libtool looks for
	  dnl " -library=stlport4 " (with the spaces either side).
	  f="-library=$l -library=Crun"
	  XAPIAN_TEST_CXXFLAGS([$f], [ISO_CXXFLAGS], [break])
	done
	dnl Older versions of this compiler defaulted to a non-standard
	dnl lifetime for temporary objects, and -features=tmplife is required
	dnl for standard conforming behaviour (which Xapian code expects).
	XAPIAN_TEST_CXXFLAGS([-features=tmplife], [ISO_CXXFLAGS])
	dnl Sun's C++ compiler warns about functions failing to return a value even
	dnl when the function ends with a "throw" statement.  That's just unhelpful
	dnl noise, and adding a dummy "return" after "throw" seems a worse solution
	dnl than simply disabling this warning - any genuinely missing return
	dnl statements will get reported by compilers with superior warning
	dnl machinery.
	XAPIAN_TEST_CXXFLAGS([-erroff=voidretw], [WARNING_CXXFLAGS])
	;;
      *)
	case `$CXX -v 2>&1` in
	MIPSpro*)
	  dnl On stderr: MIPSpro Compilers: Version 7.4.2m
	  dnl Select ANSI mode for SGI's CC with "-LANG:std".  Also, we have to
	  dnl specify "-ptused" or we get strange template linking errors.
	  ISO_CXXFLAGS="-LANG:std -ptused" ;;
	esac
	;;
      esac
      ;;
  esac

  if test -n "$ISO_CXXFLAGS" ; then
    AC_MSG_RESULT([$ISO_CXXFLAGS])
  else
    dnl Perhaps we should try to compile some code which uses iostream, for
    dnl scoping, etc, but maintaining such a snippet to cover everything we
    dnl require which any C++ compiler may not support is tricky.
    AC_MSG_RESULT([none known for $CXX])
  fi
fi

dnl Preserve the default CXXFLAGS.
save_CXXFLAGS=$CXXFLAGS

dnl Put any flags for ANSI mode in CXXFLAGS for the duration of configure
dnl since we want to make feature tests using those flags.
if test -n "$ISO_CXXFLAGS" ; then
  CXXFLAGS="$ISO_CXXFLAGS $CXXFLAGS"
  AM_CXXFLAGS="$ISO_CXXFLAGS $AM_CXXFLAGS"
fi

dnl Make 'xapian-config --cxxflags' turn on any such flags for code linking
dnl with Xapian.
AC_SUBST([ISO_CXXFLAGS])

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

dnl Some versions of Sun's C++ compiler reportedly need an explicit -lm.
dnl The maths functions we use include: exp log ceil fabs sqrt
AC_MSG_CHECKING([if -lm is required for maths functions])
dnl Don't use constant arguments as the compiler might simply evaluate the
dnl whole expression at compile time, and it might inline certain functions
dnl so test several functions.  Also write results using printf() so that
dnl the compiler can't optimise away the computations.
AC_TRY_LINK([#ifdef __SUNPRO_CC
#error Need -lm for Sun C++ 5.9 under libtool 2.2.10
#endif
#include <cmath>
#include <cstdio>
#include <ctime>
using namespace std;], [
  double a = log(ceil(time(NULL)/7.0));
  printf("%f %f %f\n", a, sqrt(a), exp(fabs(a - 12345.6)));
  ], [AC_MSG_RESULT([no])], [
    LIBS="-lm $LIBS"
    AC_TRY_LINK([#include <cmath>
#include <cstdio>
#include <ctime>
using namespace std;], [
  double a = log(ceil(time(NULL)/7.0));
  printf("%f %f %f\n", a, sqrt(a), exp(fabs(a - 12345.6)));],
	[AC_MSG_RESULT([yes])],
	[AC_MSG_ERROR([Failed to link a C++ program using <cmath>])
    ])
])

dnl See if <typeinfo> can be used in the testsuite - at least for GCC and xlC,
dnl compilation of the test code below fails if RTTI isn't being generated
dnl (g++ -fno-rtti, or by default with xlC).
AC_MSG_CHECKING([if RTTI is supported])
save_CXXFLAGS=$CXXFLAGS
dnl xlC issues a warning for typeid() being used without RTTI being enabled,
dnl so for this test we pass the xlC option to make that warning into an error.
XAPIAN_TEST_CXXFLAGS([-qhaltonmsg=1540-2412], [CXXFLAGS])
AC_TRY_COMPILE([
#include <exception>
#include <typeinfo>],
	[
	int f();
	try {
	    return f();
	} catch (std::exception & e) {
	    return typeid(e).name()[0];
	}],
    AC_MSG_RESULT([yes])
    AC_DEFINE([USE_RTTI], [1], [Define if the testsuite can use RTTI]),
    AC_MSG_RESULT([no]))
CXXFLAGS=$save_CXXFLAGS

dnl Check for time functions.
AC_CHECK_FUNCS([sleep gettimeofday ftime])

case $host_os in
  *mingw*)
    dnl For _ftime64() on mingw we need to tell it we're happy to require
    dnl MSVCRT 6.10 or higher, which isn't too onerous a requirement it seems.
    AC_DEFINE([__MSVCRT_VERSION__], [0x0601], [Define on mingw to the minimum msvcrt version to assume])
    AC_DEFINE([MINGW_HAS_SECURE_API], [1], [Define on mingw to get _s suffixed "secure" functions declared in headers])
    ;;
esac

dnl Used by tests/soaktest/soaktest.cc
AC_CHECK_FUNCS([srandom random])

dnl Used by tests/harness/testsuite.cc
AC_CHECK_FUNCS([sigaction])
AC_MSG_CHECKING([for sigsetjmp and siglongjmp])
AC_TRY_COMPILE([#include <setjmp.h>],
  [sigjmp_buf jb; if (sigsetjmp(jb, 1)) { siglongjmp(jb, 1);  }],
  AC_DEFINE([HAVE_SIGSETJMP], [1], [Define to 1 if you have the `sigsetjmp' function])
  AC_MSG_RESULT([yes]),
  AC_MSG_RESULT([no]))

dnl Used by tests/harness/cputimer.cc:
AC_CHECK_FUNCS([getrusage times sysconf])

dnl Used by tests/api_replicate.cc, tests/queryparsertest.cc and
dnl tests/termgentest.cc.
dnl
dnl Solaris < 10 only has putenv().
dnl
dnl Microsoft have marked putenv() as deprecated, so we use _putenv_s() under
dnl MSVC to avoid deprecation warnings.  We probe here since mingw doesn't
dnl currently (v3.20) provide _putenv_s(), so we can't just use it conditional
dnl on __WIN32__ being defined.
AC_CHECK_FUNCS([setenv _putenv_s])

dnl Check for a more efficient way of closing fds during daemonisation.
dnl Apparently closefrom() is available on at least "Solaris 9 or later, NetBSD
dnl 3.0 or later, OpenBSD 3.5 or later".  If we don't have closefrom(), then
dnl dirfd() and getrlimit are useful for an efficient implementation on some
dnl platforms.
AC_CHECK_FUNCS([closefrom dirfd getrlimit])

dnl See if ftime returns void (as it does on mingw)
AC_MSG_CHECKING([return type of ftime])
if test $ac_cv_func_ftime = yes ; then
  AC_TRY_COMPILE([#include <sys/timeb.h>],
    [struct timeb tp; int i = ftime(&tp);],
    AC_MSG_RESULT([int]),
    AC_MSG_RESULT([void])
    AC_DEFINE([FTIME_RETURNS_VOID], [1], [Define if ftime returns void]))
fi

dnl Check how to find the hostname: uname() in sys/utsname.h, or gethostname()
dnl Don't use default includes as inttypes.h is found by Compaq C but not C++
dnl so it causes all header probes to fail.
AC_CHECK_HEADERS([sys/utsname.h], [], [], [ ])
AC_CHECK_FUNCS([gethostname])

dnl mingw (for instance) lacks ssize_t
AC_TYPE_SSIZE_T

AC_TYPE_PID_T

AC_TYPE_MODE_T

AC_CHECK_SIZEOF([int])
AC_CHECK_SIZEOF([long])
AC_CHECK_SIZEOF([long long])

AC_CHECK_HEADERS([sys/types.h])
AC_CHECK_SIZEOF([off_t])

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

dnl Used to avoid undefined behaviour left shifting file sizes.
AC_MSG_CHECKING([for unsigned equivalent of off_t])
case $ac_cv_sizeof_off_t in
    "$ac_cv_sizeof_int")       UNSIGNED_OFF_T=unsigned ;;
    "$ac_cv_sizeof_long")      UNSIGNED_OFF_T='unsigned long' ;;
    "$ac_cv_sizeof_long_long") UNSIGNED_OFF_T='unsigned long long' ;;
    *)
      AC_MSG_RESULT([none found])
      AC_MSG_ERROR([No unsigned type equivalent to off_t found])
esac
AC_DEFINE_UNQUOTED([UNSIGNED_OFF_T], [$UNSIGNED_OFF_T], [Define to an unsigned type equivalent to off_t])
AC_MSG_RESULT([$UNSIGNED_OFF_T])

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
  dnl Checks for dot.  (Diagrams in the documentation)
  AC_PATH_PROG([DOT], [dot])
  test -z "$DOT" && AC_MSG_ERROR([dot (part of the graphviz package) is required to build documentation])
  DOXYGEN_DOT_PATH=`echo "$DOT" | sed 's!/dot$!!'`
  AC_SUBST([DOXYGEN_DOT_PATH])

  dnl FIXME: pdflatex is needed to make some more of the documentation but
  dnl doxygen expects it to be on PATH.

  dnl Check for makeindex. (Needed to make some more of the documentation)
  AC_PATH_PROG([MAKEINDEX], [makeindex], [])
  test -z "$MAKEINDEX" && AC_MSG_ERROR([makeindex is required to build documentation])

  dnl Check for doxygen. (Needed to make some more of the documentation)
  AC_PATH_PROG([DOXYGEN], [doxygen], [])
  test -z "$DOXYGEN" && AC_MSG_ERROR([doxygen is required to build documentation])

  dnl Check for help2man. (Needed to make man pages from "--help" output).
  AC_PATH_PROG([HELP2MAN], [help2man], [])
  test -z "$HELP2MAN" && AC_MSG_ERROR([help2man is required to build documentation])

  dnl Check for rst2html. (Needed to make HTML from reStructuredText format)
  dnl Also look for rst2html.py, which archlinux reportedly installs it as.
  AC_PATH_PROGS([RST2HTML], [rst2html rst2html.py], [])
  test -z "$RST2HTML" && AC_MSG_ERROR([rst2html is required to build documentation])

  dnl Check for pngcrush, which we optionally use to reduce the size of the
  dnl PNG files which doxygen generates.  We can get by without it, so don't
  dnl fail here if it's not found.
  AC_PATH_PROGS([PNGCRUSH], [pngcrush], [])
fi

dnl Check whether we need -ldl for dlsym() etc.
dnl No longer used...
dnl AC_TRY_LINK_FUNC([dlsym], ,
dnl	[AC_CHECK_LIB([dl], [dlsym], [DL_LIBS="-ldl"])])
dnl AC_SUBST([DL_LIBS])

dnl Checks for header files.
AC_CHECK_HEADERS([fcntl.h limits.h sys/errno.h sys/select.h], [], [], [ ])
AC_CHECK_HEADERS([sys/resource.h],
		 [], [], [#include <sys/types.h>])

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

dnl If eatmydata is installed, we run the testsuite under it to speed it up.
dnl If EATMYDATA is set to an empty value, then skip this check and don't use
dnl eatmydata.
if test -n "${EATMYDATA-unset}" ; then
  AC_PATH_PROG([EATMYDATA], [eatmydata], [])
fi

dnl Checks for library functions.
AC_FUNC_MEMCMP
AC_CHECK_FUNCS([hstrerror])
AC_FUNC_STRERROR_R
AC_CHECK_DECLS([sys_errlist, _sys_errlist, sys_nerr, _sys_nerr])

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

dnl ***************************
dnl * Select modules to build *
dnl ***************************

dnl Check which database backends should be built.

AC_DEFUN([XAPIAN_BACKEND_ENABLE],
  [AC_ARG_ENABLE([backend_$1],
    [AS_HELP_STRING([--enable-backend-$1], [build the $1 database backend [default=yes]])],
    [case $enableval in
      yes|no) ;;
      *) AC_MSG_ERROR([Invalid option: '--enable-backend-$1=$enableval']) ;;
    esac], [enable_backend_$1=yes])
  ])

dnl When adding a new backend, update INSTALL too.
XAPIAN_BACKEND_ENABLE([brass])
XAPIAN_BACKEND_ENABLE([chert])
XAPIAN_BACKEND_ENABLE([flint])
XAPIAN_BACKEND_ENABLE([inmemory])
XAPIAN_BACKEND_ENABLE([remote])

if test yes = "$enable_backend_brass" ; then
  dnl We would need to implement a locking strategy for MSDOS before we
  dnl could build brass there.
  case $host_os in
    *djgpp* | *msdos* ) enable_backend_brass=no ;;
  esac
fi

if test yes = "$enable_backend_chert" ; then
  dnl We would need to implement a locking strategy for MSDOS before we
  dnl could build chert there.
  case $host_os in
    *djgpp* | *msdos* ) enable_backend_chert=no ;;
  esac
fi

if test yes = "$enable_backend_flint" ; then
  dnl We would need to implement a locking strategy for MSDOS before we
  dnl could build flint there.
  case $host_os in
    *djgpp* | *msdos* ) enable_backend_flint=no ;;
  esac
fi

use_win32_uuid_api=0
case $enable_backend_flint$enable_backend_chert$enable_backend_brass in
*yes*)
  dnl We use zlib for compressing tags in brass/chert/flint.  We could
  dnl automatically disable support if zlib isn't found, but overall that
  dnl probably does more harm than good - it's most likely that someone just
  dnl forgot to install the -dev package for zlib.
  dnl
  dnl Similarly for uuid support.

  dnl Check for zlib.h.
  AC_CHECK_HEADERS([zlib.h], [], [
    AC_MSG_ERROR([zlib.h not found - required for brass, chert and flint (you may need to install the zlib1g-dev or zlib-devel package)])
    ], [ ])

  dnl Check for zlibVersion in -lz.
  SAVE_LIBS=$LIBS
  dnl mingw build needs -lzlib or -lzdll.
  AC_SEARCH_LIBS([zlibVersion], [z zlib zdll], [], [
    AC_MSG_ERROR([zlibVersion() not found in -lz, -lzlib, or -lzdll - required for brass, chert and flint (you may need to install the zlib1g-dev or zlib-devel package)])
    ])
  if test x != x"$LIBS" ; then
    XAPIAN_LIBS="$XAPIAN_LIBS $LIBS"
  fi
  LIBS=$SAVE_LIBS

  dnl Find the UUID library (from e2fsprogs/util-linux-ng, not the OSSP one).

  case $host_os in
    *mingw* | *cygwin*) dnl Use built-in API.
      use_win32_uuid_api=1
      ;;
    *)
      dnl Check for uuid/uuid.h (e2fsprogs/util-linux-ng) or uuid.h
      dnl (FreeBSD/NetBSD).
      AC_CHECK_HEADERS([uuid/uuid.h], [
	dnl util-linux-ng's uuid/uuid.h found - check for uuid_generate in
	dnl -luuid.
	SAVE_LIBS=$LIBS
	AC_SEARCH_LIBS([uuid_generate], [uuid], [], [
	  AC_MSG_ERROR([uuid_generate() not found in -luuid - required for brass, chert and flint (you may need to install the uuid-dev, libuuid-devel or e2fsprogs-devel package)])
	  ])
	if test x != x"$LIBS" ; then
	  XAPIAN_LIBS="$XAPIAN_LIBS $LIBS"
	fi
      ], [
	dnl Try uuid.h as found on FreeBSD/NetBSD, with associated code in libc.
	AC_CHECK_HEADERS([uuid.h], [
	  dnl Check for uuid_create with no extra libraries required.
	  AC_CHECK_FUNC([uuid_create], [], [
	    AC_MSG_ERROR([uuid.h found, but uuid_create() not found.  You probably wants to install libuuid from util-linux-ng or e2fsprogs (you may need to install the uuid-dev, libuuid-devel or e2fsprogs-devel package)])
	  ])
	], [
	  AC_MSG_ERROR([Neither uuid/uuid.h nor uuid.h found - required for brass, chert and flint (you may need to install the uuid-dev, libuuid-devel or e2fsprogs-devel package)])
	], [ ])
      ], [ ])
      ;;
  esac

  dnl Older versions of libuuid (such as that on CentOS 4.7) don't have
  dnl uuid_unparse_lower(), only uuid_unparse().
  AC_LINK_IFELSE([AC_LANG_CALL([], [uuid_unparse_lower])],
    [
    AC_DEFINE([HAVE_UUID_UNPARSE_LOWER], [1],
	      [Define to 1 if you have the 'uuid_unparse_lower' function.])
    ])
  LIBS=$SAVE_LIBS
  ;;
esac
AM_CONDITIONAL([USE_WIN32_UUID_API], [test "$use_win32_uuid_api" = 1])

REMOTE_LIBS=
if test "$enable_backend_remote" = yes ; then
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
      dnl We need -lws2_32 for gethostbyname(), etc.  Hardcode that knowledge
      dnl here because AC_SEARCH_LIBS fails to link the test program (I think
      dnl because its prototype has the wrong calling convention).
      REMOTE_LIBS=-lws2_32
      ;;
    *djgpp* | *msdos* )
      dnl DJGPP has a dummy implementation of fork which always fails.  If
      dnl someone actually wanted support, then DJGPP has a pthreads port,
      dnl so using threads like we do on Windows would make more sense.
      enable_backend_remote=no ;;
    *)
      dnl On Unix, we need fork and socketpair for the remotebackend.
      AC_CHECK_FUNCS([fork])
      SAVE_LIBS=$LIBS
      if test "$ac_cv_func_fork" = no ; then
	enable_backend_remote=no
      else
	dnl Check if -lsocket is required for socketpair (Solaris needs it).
	dnl And on Haiku it's in -lnetwork.
	AC_SEARCH_LIBS([socketpair], [socket network], [], [enable_backend_remote=no])
	if test "$enable_backend_remote" = yes ; then
	  AC_DEFINE([HAVE_SOCKETPAIR], [1],
		    [Define to 1 if you have the 'socketpair' function.])
	  dnl Check if -lnsl is required for gethostbyname (Solaris needs it).
	  AC_SEARCH_LIBS([gethostbyname], [nsl], [], [enable_backend_remote=no])
	  if test "$enable_backend_remote" = yes ; then
	    REMOTE_LIBS=$LIBS
	  fi
	fi
      fi
      LIBS=$SAVE_LIBS
      ;;
  esac

  if test "$enable_backend_remote" = yes ; then
    TYPE_SOCKLEN_T
    XAPIAN_LIBS="$XAPIAN_LIBS $REMOTE_LIBS"
  fi
fi

AC_ARG_ENABLE([quiet],
  [AS_HELP_STRING([--enable-quiet], [enable quiet building [default=no]])],
  [case ${enableval} in
    yes|no) ;;
    *) AC_MSG_ERROR([bad value ${enableval} for --enable-quiet]) ;;
  esac])

AC_ARG_ENABLE([visibility],
  [AS_HELP_STRING([--disable-visibility], [disable use of GCC visibility])],
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
AC_SUBST([QUIET])

vpath_build=no
if test "`pwd`" != "`cd $srcdir;pwd`" ; then
  vpath_build=yes
fi
AM_CONDITIONAL([VPATH_BUILD], [test yes = "$vpath_build"])

dnl Turn off compilation of anything that we don't have the requirements for

dnl Set conditionals to specify what we compile

AM_CONDITIONAL([BUILD_BACKEND_BRASS], [test yes = "$enable_backend_brass"])
AM_CONDITIONAL([BUILD_BACKEND_CHERT], [test yes = "$enable_backend_chert"])
AM_CONDITIONAL([BUILD_BACKEND_FLINT], [test yes = "$enable_backend_flint"])
AM_CONDITIONAL([BUILD_BACKEND_INMEMORY], [test yes = "$enable_backend_inmemory"])
AM_CONDITIONAL([BUILD_BACKEND_REMOTE], [test yes = "$enable_backend_remote"])
AM_CONDITIONAL([BUILD_BACKEND_BRASS_OR_CHERT_OR_FLINT],
  [test nonono != "$enable_backend_brass$enable_backend_chert$enable_backend_flint"])

dnl Used to decide if we should use the zlib-vg.so LD_PRELOAD hack.
use_zlib_vg=no
if test -n "$VALGRIND" ; then
  case $host_os in
    linux*) use_zlib_vg=yes ;;
  esac
fi
AM_CONDITIONAL([USE_ZLIB_VG], [test yes = "$use_zlib_vg"])

dnl See if we have fdatasync, and what libraries are needed for it.
dnl We need to actually check for a declaration as OS X has a dummy
dnl implementation in the library which is not prototyped in any header.
AC_CHECK_DECL([fdatasync(int)], [
  SAVE_LIBS=$LIBS
  AC_SEARCH_LIBS([fdatasync], [rt], [XAPIAN_LIBS="$LIBS $XAPIAN_LIBS"])
  LIBS=$SAVE_LIBS
  AC_CHECK_FUNCS([fdatasync])
  ],
  [ac_cv_func_fdatasync=no],
  [#include <unistd.h>]
)

AC_CHECK_FUNCS([fsync])

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
	    AC_DEFINE_UNQUOTED([PWRITE_PROTOTYPE], [$xo_cv_pwrite_prototype],
			       [explicit prototype needed for pwrite (if any)])
	  fi
	])
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

dnl Set flags to control warnings (enable more, or disable annoying ones).
dash_d_visibility=
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
      AC_MSG_CHECKING([for version of $CXX])
      gxx_version=`echo __GNUC__.__GNUC_MINOR__.__GNUC_PATCHLEVEL__|$CXX -E -|sed '/^#/d;s/ //g'`
      AC_MSG_RESULT([GCC $gxx_version])

      case $gxx_version in
      [[012]].* | 3.0.*)
	AC_MSG_ERROR([Xapian requires GCC 3.1 or later])
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
	  dnl
	  dnl Also, xapian-core seems to hit an aliasing bug in GCC 4.2:
	  dnl   http://gcc.gnu.org/bugzilla/show_bug.cgi?id=35643
	  dnl GCC 4.1 and 4.3 produce working code.  This bug can be avoided
	  dnl by disabling aliasing with -fno-strict-aliasing, so we do that.
	  dnl See https://trac.xapian.org/ticket/449 for more details.
	  AM_CXXFLAGS="$AM_CXXFLAGS -Wstrict-null-sentinel -Wshadow -Wstrict-overflow=1 -fno-strict-aliasing" ;;
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
      compiler=intel

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
      compiler=clang

      AM_CXXFLAGS="$AM_CXXFLAGS -Wall -W -Wredundant-decls -Wpointer-arith -Wcast-qual -Wcast-align -Wno-long-long -Wformat-security -fno-gnu-keywords -Wundef -Woverloaded-virtual"
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
  dnl WARNING_CXXFLAGS is set above.
  AM_CXXFLAGS="$AM_CXXFLAGS $WARNING_CXXFLAGS"
fi

XAPIAN_TEST_LINKER_FLAG([-Bsymbolic-functions], [symbolic_functions],
  [XAPIAN_LDFLAGS="$XAPIAN_LDFLAGS $flag"])

FP_EXCESS_PRECISION=no
AC_MSG_CHECKING([whether to use SSE instructions on x86])
case $host_cpu in
  i*86)
    if test "$enable_sse" = no ; then
      AC_MSG_RESULT([no])
      FP_EXCESS_PRECISION=yes
    else
      dnl Default to sse2.
      test "$enable_sse" != yes || enable_sse=sse2
      case $compiler in
      clang|intel)
	AC_MSG_RESULT([yes (configure with --disable-sse to disable)])
	dnl These options are supported at least as far back as clang 3.0.
	AM_CXXFLAGS="$AM_CXXFLAGS -mfpmath=sse -m$enable_sse -mtune=generic"
	;;
      gcc)
	AC_MSG_RESULT([yes (configure with --disable-sse to disable)])
	case $gxx_version in
	3.*|4.[[01]].*) dnl -mtune=generic added in GCC 4.2.
	  AM_CXXFLAGS="$AM_CXXFLAGS -mfpmath=sse -m$enable_sse" ;;
	*)
	  AM_CXXFLAGS="$AM_CXXFLAGS -mfpmath=sse -m$enable_sse -mtune=generic" ;;
	esac
	;;
      sun)
	AC_MSG_RESULT([yes (configure with --disable-sse to disable)])
	AM_CXXFLAGS="$AM_CXXFLAGS -xarch=$enable_sse"
	;;
      *)
        AC_MSG_RESULT([don't know how to for compiler $CXX])
	;;
      esac
    fi
    ;;
  *)
    AC_MSG_RESULT([non-x86 arch ($host_cpu)]) ;;
esac
AC_SUBST([FP_EXCESS_PRECISION])

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

/* With Sun CC 5.13 (Studio 12.4) on Solaris 11.2, <math.h> seems to get
 * implicitly included somehow before <cmath>, and compilation fails due
 * to 'std::exception' colliding with 'struct exception'.  It's not clear
 * how to avoid this, so just define the making macro which <cmath> does
 * before it includes <math.h>.
 */
#ifdef __SUNPRO_CC
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

dnl Split up the version number into "MAJOR.MINOR.REVISION".
MAJOR_VERSION=`echo "$VERSION"|sed 's/\..*//'`
[MINOR_VERSION=`echo "$VERSION"|sed 's/[^.]*\.//;s/\..*//'`]
[REVISION=`echo "$VERSION"|sed 's/.*\.//;s/_.*$//'`]

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
AC_CONFIG_FILES([tests/runtest], [chmod +x tests/runtest])
AC_CONFIG_FILES([tests/runsrv], [chmod +x tests/runsrv])
AC_CONFIG_FILES([tests/submitperftest], [chmod +x tests/submitperftest])
AC_CONFIG_FILES([tests/perftest/get_machine_info], [chmod +x tests/perftest/get_machine_info])
AC_CONFIG_FILES([xapian-config], [chmod +x xapian-config])
AC_CONFIG_FILES([makemanpage], [chmod +x makemanpage])
AC_CONFIG_FILES([docs/gen_codestructure_doc], [chmod +x docs/gen_codestructure_doc])
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

dnl MAIN_VERSION is VERSION without any _git123 suffix.
MAIN_VERSION="$MAJOR_VERSION.$MINOR_VERSION.$REVISION"
cxxcpp_flags=-I.
for backend in BRASS CHERT FLINT INMEMORY REMOTE ; do
  val=`eval echo "\\\$BUILD_BACKEND_${backend}_TRUE"`
  if test -z "$val" ; then
    cxxcpp_flags="$cxxcpp_flags -DXAPIAN_HAS_${backend}_BACKEND"
  fi
done
dnl For GCC (and compilers which pretend to be GCC) and MSVC, we create some
dnl ABI checks in the generated version.h, and $CXXFLAGS may contain options
dnl which affect the ABI (e.g. -fabi-version for GCC) so we want to use these
dnl options when generating version.h.  For these compilers, CXXCPP="$CXX -E"
dnl so it should understand all compiler options.
dnl
dnl For other compilers, we only pass $CPPFLAGS to $CXXCPP in case it's an
dnl external cpp which doesn't understand flags which might be in $CXXFLAGS.
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([], [[#ifndef __GNUC__
#ifndef _MSC_VER
       choke me
#endif
#endif
]])],
	[cxxcpp_flags="$cxxcpp_flags $CPPFLAGS $CXXFLAGS"],
	[cxxcpp_flags="$cxxcpp_flags $CPPFLAGS"])
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
	${SED-sed} '/"/!d;s/^ *//;/^#/d;s/ *$//;s/" *,//;s/"//g;s/@@/"/g;s/  */ /g;s/ *,\r$//;s/ *,$//'\
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
