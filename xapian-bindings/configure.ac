dnl Process this file with autoconf to produce a configure script.

dnl See xapian-core's HACKING document for details of the reasons for required
dnl versions.
AC_PREREQ([2.63])

AC_INIT([xapian-bindings], [1.2.3], [http://xapian.org/bugs])

dnl See xapian-core's HACKING document for details of the reasons for required
dnl versions.
AM_INIT_AUTOMAKE([1.10.1 -Wportability tar-ustar])
AC_CONFIG_SRCDIR([xapian.i])

AC_CONFIG_HEADERS([config.h])

AC_CONFIG_MACRO_DIR([m4])

dnl Mac OS X needs MACOSX_DEPLOYMENT_TARGET set to 10.3 (or higher) to
dnl successfully link the SWIG generated modules.  The link always fails
dnl on earlier versions of Mac OS X (10.1.4 and 10.2.8 tested), so always
dnl setting MACOSX_DEPLOYMENT_TARGET to at least 10.3 seems to be the
dnl appropriate thing to do.  Most Mac OS X developers will have upgraded
dnl from 10.1 and 10.2 by now anyhow.
dnl
dnl NB we must do this *BEFORE* AC_PROG_LIBTOOL since that is what checks
dnl MACOSX_DEPLOYMENT_TARGET.
AC_CANONICAL_HOST
OVERRIDE_MACOSX_DEPLOYMENT_TARGET=
NEED_JNILIB_EXT=no
case $host_os in
darwin*)
  case $MACOSX_DEPLOYMENT_TARGET in
  "")
    OVERRIDE_MACOSX_DEPLOYMENT_TARGET=10.3 ;;
  10.[[012]])
    AC_MSG_WARN([Overriding MACOSX_DEPLOYMENT_TARGET from $MACOSX_DEPLOYMENT_TARGET to 10.3])
    OVERRIDE_MACOSX_DEPLOYMENT_TARGET=10.3 ;;
  esac
  NEED_JNILIB_EXT=yes
  ;;
esac
AM_CONDITIONAL(OVERRIDE_MACOSX_DEPLOYMENT_TARGET, test -n "$OVERRIDE_MACOSX_DEPLOYMENT_TARGET")
AC_SUBST(OVERRIDE_MACOSX_DEPLOYMENT_TARGET)
if test -n "$OVERRIDE_MACOSX_DEPLOYMENT_TARGET" ; then
  MACOSX_DEPLOYMENT_TARGET=$OVERRIDE_MACOSX_DEPLOYMENT_TARGET
fi
AM_CONDITIONAL(NEED_JNILIB_EXT, test yes = "$NEED_JNILIB_EXT")

dnl Use libtool to manage our libraries, but don't build static libraries as
dnl the bindings only have a use for dynamic ones.
LT_PREREQ([2.2.6])
LT_INIT([disable-static])

dnl -no-undefined causes problems on Mac OS X with at least some
dnl MACOSX_DEPLOYMENT_TARGET settings, so only pass -no-undefined on
dnl platforms where it is required in order to link a shared library at
dnl all (Windows is the main one).
NO_UNDEFINED=
if test unsupported = "$allow_undefined_flag" ; then
  NO_UNDEFINED=-no-undefined
fi
AC_SUBST(NO_UNDEFINED)

dnl Checks for programs.
AC_PROG_CXX

case $host_os in
  linux* | k*bsd*-gnu)
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

dnl We want XAPIAN_CXXFLAGS to be used for configure tests, so add it to
dnl CXXFLAGS for the duration of configure.
save_CXXFLAGS=$CXXFLAGS
CXXFLAGS="$CXXFLAGS $XAPIAN_CXXFLAGS"

dnl COMPAT_VERSION is the version of xapian-core that "matches" our version.
dnl We allow for bindings being version 0.8.5.1 while xapian-core is 0.8.5.
dnl COMPAT_VERSION also ignores any _svn6789 suffix which snapshots will have.
COMPAT_VERSION=[`echo "$PACKAGE_VERSION"|sed 's/^\([0-9]*\.[0-9]*\.[0-9]*\).*/\1/'`]
AC_SUBST(COMPAT_VERSION)

dnl VERSION_NO_SNAPSHOT simply has any _svn6789 suffix removed.
VERSION_NO_SNAPSHOT=[`echo "$PACKAGE_VERSION"|sed 's/_svn[0-9]*$//'`]
AC_SUBST(VERSION_NO_SNAPSHOT)

dnl We only need to set docdir for compatibility with autoconf < 2.60 - this
dnl code can be removed once we move to requiring autoconf 2.60 or newer.
test -n "$docdir" || docdir='${datadir}/doc/${PACKAGE_TARNAME}'
AC_SUBST(docdir)

AC_ARG_ENABLE(quiet,
  [AS_HELP_STRING([--enable-quiet], [enable quiet building [default=no]])],
  [case ${enableval} in
    yes|no) ;;
    *) AC_MSG_ERROR([bad value ${enableval} for --enable-quiet]) ;;
  esac])

dnl Only probe for SWIG and enable SWIG rules in makefiles if
dnl configure --enable-maintainer-mode is used.
AM_MAINTAINER_MODE

AC_ARG_ENABLE(documentation,
  [AS_HELP_STRING([--enable-documentation], [enable make rules to rebuild documentation [default=maintainer-mode]])],
  [case ${enableval} in
    yes|no) ;;
    *) AC_MSG_ERROR([bad value ${enableval} for --enable-documentation]) ;;
  esac])
test -z "$enable_documentation" && enable_documentation=$USE_MAINTAINER_MODE
AM_CONDITIONAL(DOCUMENTATION_RULES, test x"$enable_documentation" = xyes)
AM_CONDITIONAL(MAINTAINER_NO_DOCS, test x"$USE_MAINTAINER_MODE$enable_documentation" = xyesno)

if test x$USE_MAINTAINER_MODE = xyes; then
  dnl Check for swig - this does most of the work for the bindings.
  dnl AC_PATH_PROGS only honours an already set SWIG if it's a full
  dnl path.  Listing it in the "to be searched" list like this allows
  dnl ./configure SWIG=myswig to work.
  swigoverridden=${SWIG+yes}
  AC_PATH_PROGS(SWIG, ["${SWIG-swig}"], [])
  AC_ARG_VAR(SWIG, [SWIG interface generator (only needed by Xapian developers)])
  if test -z "$SWIG" ; then
    AC_MSG_ERROR([Can't find SWIG utility])
  fi
  dnl Check for swig >= 1.3.40 (SVN rev 11194):
  dnl
  dnl 1.3.30 produces class wrappers for PHP5
  dnl 1.3.32 fixes PHP5 wrappers for overloaded forms with default arguments,
  dnl memory leaks for PHP4 and PHP5, some problems with ZTS-enabled PHP builds
  dnl (noted on Windows; Linux builds of PHP generally don't enable ZTS), and
  dnl several other issues.
  dnl 1.3.40 fixes Python 3.0.1 support and removes superfulous GIL locking to
  dnl workaround mod_python problems.
  v=`$SWIG -version 2>&1|sed 's/^SWIG Version \([[0-9\.]]*\).*/\1/p;d'`
  case $v in
    [1.3.[4-9][0-9]|1.[4-9].*|2.*]) ;;
    *)
      msg="SWIG >= 1.3.40 (SVN rev 11194 or later) required (you have ${v:-an unknown version})"
      if test -n "$swigoverridden" ; then
	dnl If SWIG was explicitly set, make this a warning only.
	AC_MSG_WARN([$msg])
      else
	AC_MSG_ERROR([$msg])
      fi ;;
  esac
  SWIG_FLAGS="`$XAPIAN_CONFIG --swigflags` -Werror"
  AC_SUBST(SWIG_FLAGS)

  dnl Need perl in maintainer mode to generate the except.i file for python.
  AC_PATH_PROGS(PERL, ["${PERL-perl}"], [])
  test -z "$PERL" && AC_MSG_ERROR([perl is required in maintainer mode])
fi

AC_ARG_WITH(python,
  AC_HELP_STRING([--with-python], [enable Python bindings]),
  [],
  [with_python=])

AC_ARG_WITH(php,
  AC_HELP_STRING([--with-php], [enable PHP bindings]),
  [],
  [with_php=])

AC_ARG_WITH(ruby,
  AC_HELP_STRING([--with-ruby], [enable Ruby bindings]),
  [],
  [with_ruby=])

AC_ARG_WITH(tcl,
  AC_HELP_STRING([--with-tcl], [enable Tcl bindings]),
  [],
  [with_tcl=])

AC_ARG_WITH(csharp,
  AC_HELP_STRING([--with-csharp], [enable CSharp bindings]),
  [],
  [with_csharp=])

AC_ARG_WITH(java,
  AC_HELP_STRING([--with-java], [enable Java bindings]),
  [],
  [with_java=])

AC_ARG_WITH(perl,
  AC_HELP_STRING([--with-perl], [enable Perl bindings]),
  [],
  [with_perl=])

case $with_python$with_php$with_ruby$with_tcl$with_csharp$with_java$with_perl in
*yes*)
  dnl Default unspecified values to no.
  test -z "$with_python" && with_python=no
  test -z "$with_php" && with_php=no
  test -z "$with_tcl" && with_tcl=no
  test -z "$with_csharp" && with_csharp=no
  test -z "$with_java" && with_java=no
  test -z "$with_ruby" && with_ruby=no
  test -z "$with_perl" && with_perl=no
  ;;
esac

BINDINGS=

if test no != "$with_python" ; then
  dnl See comment for AC_PATH_PROGS(SWIG, ...).
  AC_PATH_PROGS(PYTHON, ["${PYTHON-python}"], [])
  AC_ARG_VAR(PYTHON, [Python interpreter])
  if test -n "$PYTHON" ; then
    dnl Require Python 2.3 or newer.  It's now > 4.5 years since the last
    dnl 2.2.x release, so 2.2 is essentially unsupported upstream.
    AC_MSG_CHECKING([$PYTHON version])
    version=`$PYTHON -c 'import sys;print("%d.%d" % sys.version_info[[:2]])' 2>/dev/null`
    case $version in
    [2.[3-9]*|2.[12][0-9][0-9]*|[3-9].*])
      ;;
    *)
      AC_MSG_RESULT([$version (too old)])
      if test yes = "$with_python" ; then
	AC_MSG_ERROR([Only python 2.3 or newer is supported ($python is $version)])
      fi
      PYTHON=
      ;;
    esac
    AC_MSG_RESULT([$version])
    if test -n "$PYTHON" ; then
      if $PYTHON -c 'import distutils.sysconfig' 2>/dev/null ; then
	PYTHON_INC=`$PYTHON -c 'import os,distutils.sysconfig;print(distutils.sysconfig.get_python_inc().replace(os.sep,"/"))'`
	AC_SUBST(PYTHON_INC)
      else
	if test yes = "$with_python" ; then
	  AC_MSG_ERROR([Couldn't import Python module distutils.sysconfig - you probably need to install a python-dev or python-devel package])
	else
	  AC_MSG_WARN([Couldn't import Python module distutils.sysconfig - you probably don't have a python-dev or python-devel package installed])
	  PYTHON=
	fi
      fi
    fi
    if test -n "$PYTHON" ; then
      dnl Check that Python.h is there, which is a good way to check that
      dnl the appropriate python-dev package has been installed.
      AC_MSG_CHECKING([for $PYTHON_INC/Python.h])
      if test -f "$PYTHON_INC/Python.h" ; then
	AC_MSG_RESULT(yes)
	AC_MSG_CHECKING([for directory to install python bindings in])
	if test -z "$PYTHON_LIB" ; then
	  PYTHON_LIB=`$PYTHON -c 'import os,distutils.sysconfig;print(distutils.sysconfig.get_python_lib(1).replace(os.sep,"/"))'`
	fi
	AC_MSG_RESULT([$PYTHON_LIB])
	AC_ARG_VAR(PYTHON_LIB, [Directory to install python bindings in])

	AC_MSG_CHECKING([for python libraries to link against])
	case $host_os in
	mingw* | pw32*)
	  PYTHON_LIBS=`$PYTHON -c 'import os,sys;print("-L"+os.path.join(sys.prefix,"libs").replace(os.sep,"/")+" -lpython"+sys.version[[:3]].replace(".",""))'` ;;
	cygwin*)
	  PYTHON_LIBS=`$PYTHON -c 'import os,sys;print("-L"+os.path.join(sys.path[[3]],"config")+" -lpython"+sys.version[[:3]]')` ;;
	*)
	  PYTHON_LIBS= ;;
	esac
	AC_SUBST(PYTHON_LIBS)
	AC_MSG_RESULT([$PYTHON_LIBS])

	AC_MSG_CHECKING([for python module extension])
	dnl Usually ".so", but for example, Mac OS X uses ".dylib".
	PYTHON_SO=`$PYTHON -c 'import distutils.sysconfig;print(distutils.sysconfig.get_config_vars("SO")[[0]])'`
	AC_SUBST(PYTHON_SO)
	AC_MSG_RESULT([$PYTHON_SO])

	BINDINGS="$BINDINGS python"
      else
	AC_MSG_RESULT([no (install python-dev or python-devel package or similar)])
	if test yes = "$with_python" ; then
	  AC_MSG_ERROR([Python.h not found])
	fi
      fi
    fi
  fi
fi

if test no != "$with_php" ; then
  dnl See comment for AC_PATH_PROGS(SWIG, ...).
  AC_PATH_PROGS(PHP_CONFIG, ["${PHP_CONFIG-php-config}"], [])
  AC_ARG_VAR(PHP_CONFIG, [php-config utility])
  AC_ARG_VAR(PHP, [php interpreter (optional - only needed to run PHP testsuite with)])
  if test -n "$PHP_CONFIG" ; then
    AC_MSG_CHECKING([$PHP_CONFIG version])
    version=`$PHP_CONFIG --version 2>/dev/null`
    case $version in
    5.*)
      AC_MSG_RESULT([$version])
      PHP_MAJOR_VERSION=5
      ;;
    *)
      AC_MSG_RESULT([$version (not supported)])
      if test yes = "$with_php" ; then
	AC_MSG_ERROR([Only PHP5 is supported ($PHP_CONFIG reports version $version)])
      fi
      with_php=no
      ;;
    esac
    AC_SUBST(PHP_MAJOR_VERSION)

    if test no != "$with_php" ; then
      AC_MSG_CHECKING([for PHP extension directory])
      if test -z "$PHP_EXTENSION_DIR" ; then
	PHP_EXTENSION_DIR=`$PHP_CONFIG --extension-dir`
      fi
      AC_MSG_RESULT([$PHP_EXTENSION_DIR])
      AC_ARG_VAR(PHP_EXTENSION_DIR, [Directory to install PHP extensions in])
    fi

    if test no != "$with_php" ; then
      PHP_INC=`$PHP_CONFIG --includes`
      AC_SUBST(PHP_INC)
      save_CPPFLAGS=$CPPFLAGS
      CPPFLAGS="$CPPFLAGS $PHP_INC"
      dnl One of SGI's SuSE boxes has php-config but no headers.
      AC_CHECK_HEADER(zend.h, [], [
	if test yes = "$with_php" ; then
	  AC_MSG_ERROR(['$PHP_CONFIG --includes' reports '$PHP_INC' but zend.h can't be included from there])
	fi
	with_php=no], [ ])
      CPPFLAGS=$save_CPPFLAGS
    fi

    if test no != "$with_php" ; then
      PHP_found=no
      case $PHP in
	"")
	  dnl PHP5's php-config has --php-binary which gives the path to the
	  dnl PHP interpreter.  Otherwise, try "$prefix/bin/php$N" then
	  dnl "$prefix/bin/php".
	  php_prefix=`$PHP_CONFIG --prefix 2>/dev/null`
	  php_bin=$php_prefix/bin/php
	  php_binary=
	  if $PHP_CONFIG --php_binary > /dev/null 2>&1 ; then
	    php_binary=`$PHP_CONFIG --php-binary 2>/dev/null`
	  fi
	  for php in $php_binary $php_bin$PHP_MAJOR_VERSION $php_bin ; do
	    AC_MSG_CHECKING([for $php])
	    for ac_exec_ext in '' $ac_executable_extensions; do
	      if $as_executable_p "$php$ac_exec_ext"; then
		PHP=$php$ac_exec_ext
		AC_MSG_RESULT([$PHP])
		PHP_found=yes
		break 2
	      fi
	    done
	    AC_MSG_RESULT([no])
	  done
	  ;;
	[[\\/]* | ?:[\\/]*]) ;;
	*)
	  dnl Handle PHP passed in without a path - e.g. PHP=php5
	  AC_PATH_PROGS(PHP, ["$PHP"], [])
	  test -n "$PHP" && PHP_found=yes
	  ;;
      esac
      if test yes != "$PHP_found" ; then
	dnl Try phpN then php.  If the PHP interpreter can't be found, skip the
	dnl PHP bindings tests.
	AC_PATH_PROGS(PHP, [php$PHP_MAJOR_VERSION php], [\$(top_srcdir)/skiptest])
      fi
      if test -z "$PHP" ; then
	AC_MSG_WARN([No PHP interpreter found - PHP bindings tests will be skipped])
      fi

      case $host_os in
      mingw* | pw32*)
	dnl This is a bit of an informed guess, pending more information from
	dnl someone who actually has mingw and wants to build the PHP bindings
	dnl there.  FIXME.
	PHP_LIBS="-L`$PHP_CONFIG --prefix` -lphp${PHP_MAJOR_VERSION}ts" ;;
      cygwin*)
	PHP_LIBS="-lphp${PHP_MAJOR_VERSION}lib" ;;
      *)
	PHP_LIBS= ;;
      esac
      AC_SUBST(PHP_LIBS)
      BINDINGS="$BINDINGS php"
    fi
  fi
fi

if test no != "$with_tcl" ; then
  AC_PATH_PROGS(TCLSH, ["${TCLSH-tclsh}"], [])
  AC_ARG_VAR(TCLSH, [Tcl interpreter])
  if test -n "$TCLSH" ; then
    dnl We need Tcl 8.1 or later for TCL_STUBS.
    dnl Note: The bindings can easily be made to work with Tcl 8.0 (just
    dnl don't pass -DUSE_TCL_STUBS when compiling) should you need that.
    AC_MSG_CHECKING([$TCLSH version])
    if echo 'if {$tcl_version < 8.1 } { exit 1 }'|$TCLSH 2> /dev/null ; then
      tcl_version=`echo 'puts "$tcl_version"'|$TCLSH`
      AC_MSG_RESULT([$tcl_version])

      AC_ARG_VAR(TCL_LIB, [Directory to install Tcl files into])
      if test -z "$TCL_LIB" ; then
	[TCL_LIB=`echo 'foreach d $tcl_pkgPath {if {![regexp {/share(/|$)} $d]} {break}};puts $d'|$TCLSH`]
      fi
      AC_SUBST(TCL_LIB)

      dnl There's no obvious way to get "/usr/include/tcl8.N" directly, so
      dnl first we find the prefix Tcl is installed under (e.g. "/usr") from
      dnl the path on TCLSH (or default to /usr if the path doesn't contain
      dnl "/bin/"), and then we add "/include/tcl" and $tcl_version.
      tcl_prefix=`echo "$TCLSH"|sed 's!/bin/.*!!p;d'`
      test -n "$tcl_prefix" || tcl_prefix=/usr
      TCL_INC=${tcl_prefix}/include/tcl${tcl_version}

      dnl Check that the headers are there (the tcl8.N-dev package
      dnl may not be installed).
      TCL_CPPFLAGS=
      AC_MSG_CHECKING([for tcl.h for version $tcl_version])
      if test -f "$TCL_INC/tcl.h" ; then
	AC_MSG_RESULT([$TCL_INC/tcl.h])
	BINDINGS="$BINDINGS tcl8"
	TCL_CPPFLAGS="-I$TCL_INC"
      elif test -f "/usr/include/tcl.h" ; then
	tcl_hdr_version=`awk '($1 == "#define" && $2 == "TCL_VERSION") {print $3}' /usr/include/tcl.h`
	if test x"$tcl_hdr_version" = x'"'"$tcl_version"'"' ; then
	  AC_MSG_RESULT([/usr/include/tcl.h])
	  BINDINGS="$BINDINGS tcl8"
	else
	  AC_MSG_RESULT([not found])
	  if test yes = "$with_tcl" ; then
	    AC_MSG_ERROR([Can't find tcl.h for version $tcl_version])
	  fi
	fi
      else
	AC_MSG_RESULT([not found])
	if test yes = "$with_tcl" ; then
	  AC_MSG_ERROR([Can't find tcl.h])
	fi
      fi

      dnl Cygwin needs to link with -ltcl8.5 or similar.
      TCL_LIBS=
      case $host_os in
      cygwin*)
	AC_CHECK_LIB([tcl$tcl_version], [Tcl_Init],
		     [TCL_LIBS="-ltcl$tcl_version"],
		     [AC_MSG_ERROR([libtcl$tcl_version not found])])
	;;
      *)
	TCL_CPPFLAGS="$TCL_CPPFLAGS -DUSE_TCL_STUBS" ;;
      esac
      AC_SUBST(TCL_CPPFLAGS)
      AC_SUBST(TCL_LIBS)
    else
      AC_MSG_RESULT([< 8.1 (too old)])
      if test yes = "$with_tcl" ; then
	AC_MSG_ERROR([$TCLSH too old (Tcl 8.1 or newer required)])
      fi
    fi
  else
    if test yes = "$with_tcl" ; then
      AC_MSG_ERROR([tclsh not found])
    fi
  fi
fi

if test no != "$with_csharp" ; then
  csc_note=
  AC_ARG_VAR(CSC, [CSharp compiler command])
  if test -n "$CSC" ; then
    AC_PATH_PROGS(CSC, ["$CSC"], [])
  fi
  if test -z "$CSC" ; then
    dnl gmcs is the newer name for the Mono CSharp compiler.
    dnl cscc is the Portable.NET CSharp compiler.
    AC_PATH_PROGS(CSC, [gmcs cscc], [])
    if test -z "$CSC" ; then
      dnl csc is the Microsoft CSharp compiler.
      AC_PATH_PROGS(CSC, [csc], [])
      if test -n "$CSC" ; then
	dnl Chicken (the Scheme-to-C compiler) includes a tool called csc so we
	dnl check if the output from "csc -version" includes the word chicken
	dnl which is sufficient to distinguish Chicken's csc from Microsoft's
	dnl csc.exe.
	AC_MSG_CHECKING([whether $CSC is for CSharp])
	csc_tmp=`$CSC -version 2>/dev/null|grep chicken`
	if test -z "$csc_tmp" ; then
	  AC_MSG_RESULT(yes)
	else
	  AC_MSG_RESULT(no)
	  CSC=
	  csc_note="$csc_note (found Chicken csc program)"
	fi
      fi
    fi
    if test -z "$CSC" ; then
      dnl mcs is the Mono CSharp compiler.
      AC_PATH_PROGS(CSC, [mcs], [])
      if test -n "$CSC" ; then
	dnl There are other tools called mcs (such as /usr/bin/mcs on Tru64),
	dnl so we check that the mcs we found understands --version which is
	dnl sufficient to distinguish mono's mcs from the Tru64 one.
	AC_MSG_CHECKING([whether $CSC is from GNU Mono])
	if (exec >&5 2>&5;$CSC --version </dev/null;exit $?) ; then
	  AC_MSG_RESULT(yes)
	else
	  AC_MSG_RESULT(no)
	  CSC=
	  csc_note="$csc_note (found different mcs program)"
	fi
      fi
    fi
  fi

  if test -n "$CSC" ; then
    AC_PATH_PROGS(GACUTIL, ["${GACUTIL-gacutil}"], [])
    if test -z "$GACUTIL" ; then
      AC_PATH_PROGS(GACUTIL, [ilgac], [])
    fi
    AC_PATH_PROGS(SN, ["${SN-sn}"], [])
    AC_ARG_VAR(GACUTIL, [gacutil utility to use for CSharp bindings])
    AC_ARG_VAR(SN, [sn utility to use for CSharp bindings])
    if test -z "$SN" ; then
      case $GACUTIL in
      dnl Portable.net doesn't support strong naming yet and ilgac doesn't
      dnl require assemblies to be strong named before adding them to the GAC.
      *ilgac*) SN=/bin/true ;;
      esac
    fi
    if test -n "$GACUTIL" -a -n "$SN" ; then
      AC_MSG_CHECKING([whether the CSharp compiler works])
      [echo 'class conftest { public static void Main() { System.Console.WriteLine("OK"); } }' > conftest.cs]
      if (exec >&5 2>&5;$CSC /out:conftest.exe conftest.cs;exit $?) ; then
	AC_MSG_RESULT(yes)
	AC_MSG_CHECKING([whether CSharp programs can just be run])
	if test OK = "`./conftest.exe 2> /dev/null`" ; then
	  AC_MSG_RESULT(yes)
	  RUN_CSHARP=
	else
	  AC_MSG_RESULT(no)
	  AC_PATH_PROGS(MONO, ["${MONO-mono}"], [])
	  if test -z "$MONO" ; then
	    AC_PATH_PROGS(MONO, [ilrun], [])
	  fi
	  AC_ARG_VAR(MONO, [CSharp bytecode interpreter (optional - only needed to run CSharp testsuite with)])
	  if test -n "$MONO" ; then
	    AC_MSG_CHECKING([whether $MONO can run CSharp programs])
	    if OK = "`$MONO ./conftest.exe 2> /dev/null`" ; then
	      AC_MSG_RESULT(yes)
	      RUN_CSHARP=mono
	    else
	      AC_MSG_RESULT([no - CSharp tests will be skipped])
	      RUN_CSHARP='\$(top_srcdir)/skiptest'
	    fi
	  else
	    AC_MSG_RESULT([not found - CSharp tests will be skipped])
	    RUN_CSHARP='\$(top_srcdir)/skiptest'
	  fi
	fi
	AC_SUBST(RUN_CSHARP)
	BINDINGS="$BINDINGS csharp"
      else
	AC_MSG_RESULT(no)
	if test yes = "$with_csharp" ; then
	  AC_MSG_ERROR([CSharp compiler $CSC doesn't work])
	fi
      fi
    else
      if test yes = "$with_csharp" ; then
	if test -z "$GACUTIL" ; then
	  AC_MSG_ERROR([Mono gacutil not found])
	elif test -z "$SN" ; then
	  AC_MSG_ERROR([Mono sn not found])
	fi
      fi
    fi
  else
    if test yes = "$with_csharp" ; then
      AC_MSG_ERROR([CSharp compiler not found$csc_note])
    fi
  fi
fi

JAVA_CPPFLAGS=
if test no != "$with_java" ; then
  AC_PATH_PROGS(JAVA, ["${JAVA-java}"], [],
	[${JAVA_HOME+$JAVA_HOME/bin:}${JDK_HOME+$JDK_HOME/bin:}$PATH])
  AC_PATH_PROGS(JAVAC, ["${JAVAC-javac}"], [],
	[${JAVA_HOME+$JAVA_HOME/bin:}${JDK_HOME+$JDK_HOME/bin:}$PATH])
  AC_PATH_PROGS(JAR, ["${JAR-jar}"], [],
	[${JAVA_HOME+$JAVA_HOME/bin:}${JDK_HOME+$JDK_HOME/bin:}$PATH])
  AC_ARG_VAR(JAVA, [Java interpreter command])
  AC_ARG_VAR(JAVAC, [Java compiler command])
  AC_ARG_VAR(JAR, [java jar utility])
  AC_ARG_VAR(JAVA_HOME, [Pathname of the directory where the JDK is installed])
  AC_ARG_VAR(JDK_HOME, [Pathname of the directory where the JDK is installed])
  AC_ARG_VAR(JNI_INCLUDE_DIR, [Pathname of the directory where jni.h is installed])
  if test -n "$JAVA" -a -n "$JAVAC" -a -n "$JAR" ; then
    dnl Eric says:
    dnl  The JNI library *requires* "Java 2", which is 1.2 or better.
    dnl
    dnl So checking for jni.h presumably implicitly checks we have "Java 2".
    dnl Note: jni.h #defines JNI_VERSION_1_[124] (but not 3 it seems).
    dnl So we could check for that if we want to check for a particular
    dnl JDK version...
    java_ok=no
    AC_CHECK_HEADER([jni.h], [java_ok=yes], [
      dnl We need to look for jni.h in /usr/lib/jvm/java-gcj/include on some
      dnl Debian and Ubuntu boxes at least.  And on Mac OS X, we jni.h lives
      dnl in /System/Library/Frameworks/JavaVM.framework/Headers.
      SAVE_CPPFLAGS=$CPPFLAGS
      for jnidir in $JNI_INCLUDE_DIR ${JAVA_HOME+"$JAVA_HOME/include"} ${JDK_HOME+"$JDK_HOME/include"} /usr/lib/jvm/java-gcj/include /System/Library/Frameworks/JavaVM.framework/Headers ; do
	CPPFLAGS="$SAVE_CPPFLAGS -I$jnidir"
	AC_MSG_CHECKING([for jni.h in $jnidir])
	if test -f "$jnidir/jni.h" ; then
	  AC_COMPILE_IFELSE([#include <jni.h>], [java_ok=yes])
	  if test yes = $java_ok ; then
	    AC_MSG_RESULT([yes])
	    JAVA_CPPFLAGS=-I$jnidir
	    break
	  fi

	  dnl Sun's JDK requires the user to pass an extra -I for a
	  dnl subdirectory named after the host_os.  Sadly we seem to
	  dnl need to hardcode a list of the platform names.
	  jni_host_os=
	  case $host_os in
	    linux*)
	      jni_host_os=linux ;;
	    solaris*)
	      jni_host_os=solaris ;;
	    mingw* | pw32*)
	      jni_host_os=win32 ;;
	  esac
	  if test -n "$jni_host_os" ; then
	    CPPFLAGS="$CPPFLAGS -I$jnidir/$jni_host_os"
	    AC_COMPILE_IFELSE([#include <jni.h>], [java_ok=yes])
	    if test yes = $java_ok ; then
	      AC_MSG_RESULT([yes, requires additional -I$jnidir/$jni_host_os])
	      JAVA_CPPFLAGS="-I$jnidir -I$jnidir/$jni_host_os"
	      break
	    fi
	  fi
	  AC_MSG_RESULT([found, but not usable])

	  CPPFLAGS="$SAVE_CPPFLAGS -I$jnidir"
	else
	  AC_MSG_RESULT([no])
	fi
      done
      CPPFLAGS=$SAVE_CPPFLAGS
      test noyes = $java_ok$with_java && AC_MSG_ERROR([jni.h not found])
      ], [ ])
    AC_SUBST(JAVA_CPPFLAGS)
    if test yes = $java_ok ; then
      AC_MSG_CHECKING([for Java path separator])
      [echo 'public class conftest { public static void main(String[] args) { System.out.println(System.getProperty("path.separator")); } }' > conftest.java]
      if (exec >&5 2>&5;$JAVAC conftest.java;exit $?) ; then
	JAVA_PATHSEP=`$JAVA conftest 2>&5`
	AC_SUBST(JAVA_PATHSEP)
	if test -n "$JAVA_PATHSEP" ; then
	  AC_MSG_RESULT($JAVA_PATHSEP)
	  BINDINGS="$BINDINGS java"
	else
	  AC_MSG_RESULT([failed to run test program])
	  java_ok=no
	fi
      else
	AC_MSG_RESULT([failed to compile test program])
	java_ok=no
      fi
      if test no = "$java_ok" ; then
	test yes = "$with_java" && AC_MSG_ERROR([Couldn't compile and run a simple Java test program])
      fi
    fi
  else
    if test yes = "$with_java" ; then
      if test -z "$JAVA" ; then
	AC_MSG_ERROR([java not found])
      elif test -z "$JAVAC" ; then
	AC_MSG_ERROR([javac not found])
      elif test -z "$JAR" ; then
	AC_MSG_ERROR([jar not found])
      fi
    fi
  fi
fi

if test no != "$with_ruby" ; then
  dnl See comment for AC_PATH_PROGS(SWIG, ...).
  if test -n "$RUBY" ; then
    AC_PATH_PROGS(RUBY, ["$RUBY"], [])
  else
    AC_PATH_PROGS(RUBY, [ruby1.8 ruby], [])
  fi
  AC_ARG_VAR(RUBY, [Ruby interpreter])
  if test -n "$RUBY" ; then
    dnl Require Ruby 1.8 or newer.  Paul says:
    dnl  I'm using 1.8.4.  I imagine anything in the 1.8.x series will be fine,
    dnl  not sure about 1.6 but hardly anyone uses <1.8.
    dnl
    dnl Brief testing with Ruby 1.6.8 show the bindings probably work there,
    dnl but smoketest.rb doesn't because the test/unit module isn't available.
    AC_MSG_CHECKING([$RUBY version])
    version=`$RUBY --version 2>/dev/null`
    case $version in
    [ruby\ 1.[89]*]) ;; # Ruby 1.8, 1.9
    [ruby\ 1.[1-9][0-9]*]) ;; # Ruby 1.10+
    [ruby\ [2-9]*]) ;; # Ruby 2-Ruby 9
    [ruby\ 1[0-9]*]) ;; # Ruby 10+
    *)
      AC_MSG_RESULT([$version (too old)])
      if test yes = "$with_ruby" ; then
	AC_MSG_ERROR([Only Ruby 1.8 or newer is supported ($RUBY is $version)])
      fi
      RUBY=
      ;;
    esac
    if test -n "$RUBY" ; then
      AC_MSG_RESULT([$version])
      AC_ARG_VAR(RUBY_INC, [Directory where ruby.h can be found])
      if test -z "$RUBY_INC" ; then
	dnl Ruby 1.9 added rubyhdrdir; for older Ruby we use archdir.
	[RUBY_INC=`$RUBY -rrbconfig -e 'puts Config::CONFIG["rubyhdrdir"] || Config::CONFIG["archdir"]'`]
      fi
      AC_SUBST(RUBY_INC)
      dnl Check that ruby.h is there, which is a good way to check that
      dnl the appropriate ruby-dev package has been installed.
      AC_MSG_CHECKING([for $RUBY_INC/ruby.h])
      if test -f "$RUBY_INC/ruby.h" ; then
	AC_MSG_RESULT(yes)

	AC_MSG_CHECKING([ruby/io.h])
	rm -f ruby/rubyio.h
	if test -f "$RUBY_INC/ruby/io.h" ; then
	  AC_MSG_RESULT(yes)
	  dnl Newer Ruby has ruby/io.h - older has rubyio.h.  Ruby doesn't
	  dnl seem to provide any way to generate code which works with
	  dnl both (and even the version number isn't a discriminator as
	  dnl Debian etch has 1.9 without ruby/io.h).  The SWIG version
	  dnl we're currently using assumes rubyio.h exists and luckily
	  dnl it includes it with "", so we can just drop a rubyio.h
	  dnl wrapper alongside ruby/xapian_wrap.cc.
	  echo '#include <ruby/io.h>' > ruby/rubyio.h
	else
	  AC_MSG_RESULT(no)
	fi

	AC_ARG_VAR(RUBY_INC_ARCH, [Directory where ruby/config.h can be found (needed from Ruby 1.9)])
	if test -z "$RUBY_INC_ARCH" ; then
	  dnl Ruby 1.9 requires this.
	  [RUBY_INC_ARCH=`$RUBY -rrbconfig -e 'd = Config::CONFIG["rubyhdrdir"];if d != nil; print d + "/" + Config::CONFIG["arch"]; end'`]
	  dnl For earlier versions, just make it the same as RUBY_INC.
	  test x"$RUBY_INC_ARCH" != x || RUBY_INC_ARCH=$RUBY_INC
	fi
	AC_SUBST(RUBY_INC_ARCH)

	AC_ARG_VAR(RUBY_LIB, [Directory to install ruby files into])
	if test -z "$RUBY_LIB" ; then
	  [RUBY_LIB=`$RUBY -rrbconfig -e 'puts Config::CONFIG["sitelibdir"]'`];
	fi
	AC_SUBST(RUBY_LIB)

	AC_ARG_VAR(RUBY_LIB_ARCH, [Directory to install ruby binary modules into])
	if test -z "$RUBY_LIB_ARCH" ; then
	  [RUBY_LIB_ARCH=`$RUBY -rrbconfig -e 'puts Config::CONFIG["sitearchdir"]'`]
	fi
	AC_SUBST(RUBY_LIB_ARCH)

	RUBY_LIBS=
	case $host_os in
	cygwin*) RUBY_LIBS=-lruby ;;
	esac
	AC_SUBST(RUBY_LIBS)

	if test yes = "$enable_documentation" ; then
	  if test -n "$RDOC" ; then
	    AC_PATH_PROGS(RDOC, ["$RDOC"], [])
	  else
	    dnl Try replace the *last* 'ruby' with 'rdoc'.
	    rdoc_best_guess=`echo "$RUBY"|sed 's,\(.*\)ruby,\1rdoc,'`
	    AC_PATH_PROGS(RDOC, ["$rdoc_best_guess"], [])
	    if test -z "$RDOC"; then
	      AC_PATH_PROGS(RDOC, [rdoc1.8 rdoc], [])
	      test -n "$RDOC" || RDOC="$MISSING rdoc"
	    fi
	  fi
	fi

	BINDINGS="$BINDINGS ruby"
      else
	AC_MSG_RESULT([no (install ruby-dev or ruby-devel package or similar)])
	if test yes = "$with_ruby" ; then
	  AC_MSG_ERROR([ruby.h not found])
	fi
      fi
      RUBY_DLEXT=`$RUBY -rrbconfig -e 'puts Config::CONFIG[["DLEXT"]]'`
      AC_SUBST(RUBY_DLEXT)
    fi
  fi
fi

if test yes = "$with_perl" ; then
  AC_PATH_PROGS(PERL, ["${PERL-perl}"], [])
  AC_ARG_VAR(PERL, [perl interpreter])
  if test -n "$PERL" ;  then
    AC_ARG_VAR(PERL_INC, [Directory to include XS headers])
    if test -z "$PERL_INC" ; then
      PERL_INC=`$PERL -MConfig -e 'print $Config{archlibexp}, "/CORE";'`
    fi
    AC_SUBST(PERL_INC)

    AC_ARG_VAR(PERL_LIB, [Directory to install perl files into])
    if test -z "$PERL_LIB" ; then
      [PERL_LIB=`$PERL -MConfig -e 'print $Config{sitearch};'`];
    fi
    AC_SUBST(PERL_LIB)

    AC_ARG_VAR(PERL_SO, [for perl module extension])
    if test -z "$PERL_SO" ; then
      [PERL_SO=`$PERL -MConfig -e 'print ".", $Config{'dlext'};'`];
    fi
    AC_SUBST(PERL_SO)
    
    if test -z "$PERL_XAPIAN_VERSION" ; then
      [PERL_XAPIAN_VERSION=$PACKAGE_VERSION.0];
    fi
    AC_SUBST(PERL_XAPIAN_VERSION)

    BINDINGS="$BINDINGS perl"
  else
    if test yes = "$with_perl" ; then
      AC_MSG_ERROR([perl not found])
    fi
  fi
fi

AC_SUBST(BINDINGS)

dnl Set flags to control warnings (enable more, or disable annoying ones)
dnl and other compiler specific flags.
SWIG_CXXFLAGS=
JAVA_CXXFLAGS=
if test yes = "$GXX" ; then
  dnl Python itself is compiled with -fno-strict-aliasing, and it appears
  dnl it's safest to follow this lead when compiling the SWIG generated
  dnl interface code.  E.g.:
  dnl   http://article.gmane.org/gmane.comp.gcc.devel/74692
  dnl The code SWIG generates for other languages seems to have similar
  dnl issues too, so just turn this on for all of them.
  dnl
  dnl There's no need to check -fno-strict-aliasing is supported as it works
  dnl with GCC 2.95 and we don't support anything older.
  SWIG_CXXFLAGS="-fno-strict-aliasing"

  dnl We need to explicitly link against -lstdc++ on OpenBSD (discovered
  dnl on OpenBSD 3.7 with GCC 3.3.5 but this appears to be due to a
  dnl deliberate decision on the part of OpenBSD developers).  Luckily
  dnl we can just always specify -lstdc++ explicitly if GCC is the
  dnl compiler and libtool will eliminate the duplicate on other
  dnl platforms.
  XAPIAN_LIBS="$XAPIAN_LIBS -lstdc++"

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
      dnl
      dnl Swig generated code gives lots of unused and uninitialized warnings.
      dnl They're non-harmful, so suppress them.
      SWIG_CXXFLAGS="$SWIG_CXXFLAGS -Wall -w1 -wd177,1572"
      dnl The Java JNI wrappers have a few unused variables.
      JAVA_CXXFLAGS="-Wall -w1 -wd177,1572"
    ],
    [
      dnl GCC:
      dnl
      dnl Swig generated code gives lots of unused and uninitialized warnings.
      dnl They're non-harmful, so suppress them.
      SWIG_CXXFLAGS="$SWIG_CXXFLAGS -Wall -Wno-unused -Wno-uninitialized"
      AC_MSG_CHECKING([whether to use -fvisibility=hidden])
      dnl To be robust, we check both that -fvisibility=hidden is recognised
      dnl and that SWIG will have added the visibility attributes.
      dnl The GCC wiki says it's supported from GCC 4.0 onwards.  GCC 3.4
      dnl also supports -fvisibility, but doesn't allow visibility to be
      dnl specified for classes, which probably breaks throwing exception
      dnl classes from the library.  We want that to work, so only switch
      dnl this on for GCC 4.0 onwards.
      SAVE_CXXFLAGS=$CXXFLAGS
      CXXFLAGS="-fvisibility=hidden"
      AC_TRY_COMPILE([], [
#if __GNUC__ >= 4
  /* GCC supports -fvisibility and allows specifying visibility for classes. */
#else
  choke me
#endif
],
		     AC_MSG_RESULT(yes)
		     SWIG_CXXFLAGS="$SWIG_CXXFLAGS -fvisibility=hidden"
		     ,
		     AC_MSG_RESULT(no))
      CXXFLAGS=$SAVE_CXXFLAGS
      dnl The Java JNI wrappers have a few unused variables.
      JAVA_CXXFLAGS="-Wall -W -Wno-unused"
    ])
fi
AC_SUBST(SWIG_CXXFLAGS)
AC_SUBST(JAVA_CXXFLAGS)

dnl Restore CXXFLAGS to those the user specified or autoconf defaulted to.
CXXFLAGS=$save_CXXFLAGS

dnl Required for auto regeneration to work in a combined maintainer-mode tree.
: ${AUTOM4TE=autom4te}
AC_SUBST([AUTOM4TE])

AC_CONFIG_FILES([Makefile
 xapian-version.h
 python/Makefile python/docs/Makefile
 php/Makefile php/docs/Makefile
 java/Makefile java/native/Makefile java/org/xapian/Makefile
 java/org/xapian/errors/Makefile java/org/xapian/examples/Makefile
 java-swig/Makefile
 tcl8/Makefile tcl8/docs/Makefile
 tcl8/pkgIndex.tcl
 csharp/Makefile csharp/docs/Makefile
 csharp/AssemblyInfo.cs
 ruby/Makefile ruby/docs/Makefile
 xapian-bindings.spec
 perl/Makefile perl/docs/Makefile perl/util.i
 ])
AC_OUTPUT

echo ""
if test -z "$BINDINGS" ; then
  echo "*** Not found the required tools for building bindings for any"
  echo "*** supported language!"
  echo "***"
  echo "*** You may need to install -dev or -devel packages for the languages"
  echo "*** you want to build bindings for."
  echo "***"
  echo "*** For details of how to point configure at tools not on PATH, see:"
  echo "***     ./configure --help"
else
  echo "*** Building bindings for languages:" $BINDINGS
fi
echo ""
if test "$COMPAT_VERSION" != "$XAPIAN_VERSION" ; then
  dnl 0.8.2 was the first version which set XAPIAN_VERSION
  AC_MSG_WARN([Xapian library is version ${XAPIAN_VERSION-<0.8.2} but the bindings are version $PACKAGE_VERSION - we strongly recommend using matching versions.])
fi
