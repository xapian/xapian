dnl Process this file with autoconf to produce a configure script.

dnl See xapian-core's HACKING document for details of the reasons for required
dnl versions.
AC_PREREQ([2.63])

m4_define([project_version], [1.4.27])
AC_INIT([xapian-bindings], project_version, [https://xapian.org/bugs])

dnl See xapian-core's HACKING document for details of the reasons for required
dnl versions.
AM_INIT_AUTOMAKE([1.12.2 parallel-tests -Wportability subdir-objects tar-ustar no-dist-gzip dist-xz])
AC_CONFIG_SRCDIR([xapian-headers.i])

AC_CONFIG_HEADERS([config.h])

AC_CONFIG_MACRO_DIR([m4])

AC_CANONICAL_HOST

dnl Use libtool to manage our libraries, but don't build static libraries as
dnl the bindings only have a use for dynamic ones.
LT_PREREQ([2.2.6])
LT_INIT([disable-static])

dnl Make the name of libtool's `.libs`/`_libs` available in makefiles.
_libs=$objdir
AC_SUBST([_libs])

dnl -no-undefined causes problems on macOS with at least some
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

dnl Run tests using the C++ compiler.
AC_LANG([C++])

dnl Probe for any options needed to enable C++11 support.
AX_CXX_COMPILE_STDCXX([11])

dnl Check for xapian-core.
XO_LIB_XAPIAN([], [],
    [xapian-config]regexp(project_version,
			  [^\([0-9]*\.[0-9]*[13579]\)\..*$], [-\1]))
XO_REQUIRE([1.4.0])

NEED_JNILIB_EXT=no
NEED_INTREE_DYLD=no
INTREE_DYLD_PATH=
case $host_os in
darwin*)
  NEED_JNILIB_EXT=yes
  dnl If we're building against an uninstalled xapian-core, xapian-config
  dnl --libs will give an error.
  if $XAPIAN_CONFIG --libs >/dev/null 2>/dev/null ; then
    :
  else
    NEED_INTREE_DYLD=yes
    dnl $XAPIAN_LIBS may have some options first, so grab the last argument,
    dnl which should be the .la file.
    set x $XAPIAN_LIBS
    as_fn_arith $# - 1
    shift $as_val
    INTREE_DYLD_PATH=$1
    dnl Make sure the path is absolute.
    case $INTREE_DYLD_PATH in
      /*) ;;
      *) INTREE_DYLD_PATH=`pwd`/$INTREE_DYLD_PATH ;;
    esac
    dnl Add .libs or equivalent.
    INTREE_DYLD_PATH=`echo "$INTREE_DYLD_PATH"|sed 's,\(.*/\).*,\1,'`$_libs
    AC_SUBST([INTREE_DYLD_PATH])
  fi
  ;;
esac
AM_CONDITIONAL(NEED_JNILIB_EXT, test yes = "$NEED_JNILIB_EXT")
AM_CONDITIONAL(NEED_INTREE_DYLD, test yes = "$NEED_INTREE_DYLD")

dnl We want XAPIAN_CXXFLAGS to be used for configure tests, so add it to
dnl CXXFLAGS for the duration of configure.
save_CXXFLAGS=$CXXFLAGS
CXXFLAGS="$CXXFLAGS $XAPIAN_CXXFLAGS"

dnl COMPAT_VERSION is the version of xapian-core that "matches" our version.
dnl We allow for bindings being version 0.8.5.1 while xapian-core is 0.8.5.
dnl COMPAT_VERSION also ignores any _git123 suffix which snapshots will have.
COMPAT_VERSION=[`echo "$PACKAGE_VERSION"|sed 's/^\([0-9]*\.[0-9]*\.[0-9]*\).*/\1/'`]
AC_SUBST(COMPAT_VERSION)

dnl VERSION_NO_SNAPSHOT simply has any _git123 suffix removed.
VERSION_NO_SNAPSHOT=[`echo "$PACKAGE_VERSION"|sed 's/_.*$//'`]
AC_SUBST(VERSION_NO_SNAPSHOT)

dnl We only need to set docdir for compatibility with autoconf < 2.60 - this
dnl code can be removed once we move to requiring autoconf 2.60 or newer.
test -n "$docdir" || docdir='${datadir}/doc/${PACKAGE_TARNAME}'
AC_SUBST(docdir)

AC_ARG_ENABLE(visibility,
  [AS_HELP_STRING([--disable-visibility], [disable use of GCC visibility])],
  [case ${enableval} in
    yes|no) ;;
    *) AC_MSG_ERROR([bad value ${enableval} for --disable-visibility]) ;;
  esac])

dnl Only probe for SWIG and enable SWIG rules in makefiles if
dnl configure --enable-maintainer-mode is used.
AM_MAINTAINER_MODE

AC_ARG_ENABLE(documentation,
  [AS_HELP_STRING([--enable-documentation], [enable make rules to rebuild shipped documentation [default=maintainer-mode]])],
  [case ${enableval} in
    yes|no) ;;
    *) AC_MSG_ERROR([bad value ${enableval} for --enable-documentation]) ;;
  esac])
test -z "$enable_documentation" && enable_documentation=$USE_MAINTAINER_MODE
AM_CONDITIONAL(DOCUMENTATION_RULES, test x"$enable_documentation" = xyes)
AM_CONDITIONAL(MAINTAINER_NO_DOCS, test x"$USE_MAINTAINER_MODE$enable_documentation" = xyesno)

if test yes = "$enable_documentation" ; then
  dnl Check for rst2html. (Needed to make HTML from reStructuredText format)
  dnl Also look for rst2html.py, which archlinux reportedly installs it as.
  AC_PATH_PROGS(RST2HTML, [rst2html rst2html.py], [])
  AC_ARG_VAR([RST2HTML], [reST to HTML convertor])
  test -z "$RST2HTML" && AC_MSG_ERROR([rst2html is required to build documentation (try package python-docutils)])
fi

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
  dnl Check for new enough SWIG.
  dnl
  dnl 4.1.0 adds support for PHP 8
  v=`$SWIG -version 2>&1|sed 's/^SWIG Version \([[0-9\.]]*\).*/\1/p;d'`
  case $v in
    [[0123].*|4.0.*|""])
      msg="SWIG >= 4.1.0 required (you have ${v:-an unknown version})"
      if test -n "$swigoverridden" ; then
	dnl If SWIG was explicitly set, make this a warning only.
	AC_MSG_WARN([$msg])
      else
	AC_MSG_ERROR([$msg])
      fi ;;
  esac
  SWIG_FLAGS=`$XAPIAN_CONFIG --swigflags`
  AC_SUBST(SWIG_FLAGS)
  XAPIAN_HEADER_DIRS=
  XAPIAN_HEADERS=
  for a in $SWIG_FLAGS ; do
    case $a in
    -I*)
      d=`echo "x$a"|sed 's/^x-I//'`
      dnl Quote each entry in XAPIAN_HEADER_DIRS with "".
      XAPIAN_HEADER_DIRS=$XAPIAN_HEADER_DIRS' "'$d'"'
      for f in "$d/"*.h "$d/xapian/*.h" ; do
	if test -f "$f" ; then
	  XAPIAN_HEADERS="$XAPIAN_HEADERS $f"
	fi
      done
      ;;
    esac
  done
  AC_SUBST([XAPIAN_HEADER_DIRS])
  AC_SUBST([XAPIAN_HEADERS])

  dnl Need perl in maintainer mode to generate the except.i file for python.
  AC_PATH_PROGS(PERL, ["${PERL-perl}"], [])
  test -z "$PERL" && AC_MSG_ERROR([perl is required in maintainer mode])
  dnl Need doxygen in maintainer mode to convert C++ doxygen comments to Python
  dnl docstrings.
  AC_PATH_PROG(DOXYGEN, doxygen, [])
  AC_ARG_VAR([DOXYGEN], [Doxygen documentation system])
  test -z "$DOXYGEN" && AC_MSG_ERROR([doxygen is required to build documentation])
fi

AC_ARG_WITH(python,
  AC_HELP_STRING([--with-python], [enable Python 2 bindings]),
  [],
  [with_python=])

AC_ARG_WITH(python3,
  AS_HELP_STRING([--with-python3], [enable Python 3 bindings]),
  [],
  [with_python3=])

AC_ARG_WITH(php,
  AS_HELP_STRING([--with-php], [enable PHP bindings]),
  [],
  [with_php=])

AC_ARG_WITH(ruby,
  AS_HELP_STRING([--with-ruby], [enable Ruby bindings]),
  [],
  [with_ruby=])

AC_ARG_WITH(tcl,
  AS_HELP_STRING([--with-tcl], [enable Tcl bindings]),
  [],
  [with_tcl=])

AC_ARG_WITH(csharp,
  AS_HELP_STRING([--with-csharp], [enable CSharp bindings]),
  [],
  [with_csharp=])

AC_ARG_WITH(java,
  AS_HELP_STRING([--with-java], [enable Java bindings]),
  [],
  [with_java=])

AC_ARG_WITH(perl,
  AS_HELP_STRING([--with-perl], [enable Perl bindings]),
  [],
  [with_perl=])

AC_ARG_WITH(lua,
  AS_HELP_STRING([--with-lua], [enable Lua bindings]),
  [],
  [with_lua=])

case $with_python$with_python3$with_php$with_ruby$with_tcl$with_csharp$with_java$with_perl$with_lua in
*yes*)
  dnl Default unspecified values to no.
  test -z "$with_python" && with_python=no
  test -z "$with_python3" && with_python3=no
  test -z "$with_php" && with_php=no
  test -z "$with_tcl" && with_tcl=no
  test -z "$with_csharp" && with_csharp=no
  test -z "$with_java" && with_java=no
  test -z "$with_ruby" && with_ruby=no
  test -z "$with_perl" && with_perl=no
  test -z "$with_lua" && with_lua=no
  ;;
esac

BINDINGS=

if test no != "$with_python" ; then
  dnl For backward compatibility:
  : ${PYTHON2="$PYTHON"}
  : ${PYTHON2_INC="$PYTHON_INC"}
  : ${PYTHON2_LIB="$PYTHON_LIB"}
  : ${PYTHON2_LIBS="$PYTHON_LIBS"}
  : ${PYTHON2_SO="$PYTHON_SO"}
  dnl See comment for AC_PATH_PROGS(SWIG, ...).
  if test -n "$PYTHON2" ; then
    AC_PATH_PROGS(PYTHON2, ["$PYTHON2"], [])
  else
    AC_PATH_PROGS(PYTHON2, [python2 python], [])
  fi
  AC_ARG_VAR(PYTHON2, [Python 2 interpreter])
  if test -n "$PYTHON2" ; then
    dnl Require Python 2.7 or newer.  Older versions are essentially
    dnl unsupported upstream, and SWIG 4.0.0 dropped support for them too.
    AC_MSG_CHECKING([$PYTHON2 version])
    version=`$PYTHON2 -c 'import sys;print("%d.%d" % sys.version_info[[:2]])' 2>/dev/null`
    case $version in
    [2.[7-9]*|2.[1-5][0-9]*])
      AC_MSG_RESULT([$version])
      ;;
    [[3-9].*])
      AC_MSG_RESULT([$version (too new - use --with-python3 for Python 3 support)])
      if test yes = "$with_python" ; then
	AC_MSG_ERROR([Use --with-python3 for Python 3 support ($PYTHON2 is $version)])
      fi
      PYTHON2=
      ;;
    *)
      AC_MSG_RESULT([$version (too old)])
      if test yes = "$with_python" ; then
	AC_MSG_ERROR([Only Python 2.7 or newer is supported ($PYTHON2 is $version)])
      fi
      PYTHON2=
      ;;
    esac
    if test -n "$PYTHON2" ; then
      if $PYTHON2 -c 'import distutils.sysconfig' 2>/dev/null ; then
	PYTHON2_INC=`$PYTHON2 -c 'import os,distutils.sysconfig;print(distutils.sysconfig.get_python_inc().replace(os.sep,"/"))'`
	AC_SUBST(PYTHON2_INC)
      else
	if test yes = "$with_python" ; then
	  AC_MSG_ERROR([Couldn't import Python module distutils.sysconfig - you probably need to install a python-dev or python-devel package])
	else
	  AC_MSG_WARN([Couldn't import Python module distutils.sysconfig - you probably don't have a python-dev or python-devel package installed])
	  PYTHON2=
	fi
      fi
    fi
    AC_MSG_CHECKING([for python2 sphinx module])
    if $PYTHON2 -c 'import sphinx;print(repr(sphinx.main))' >&5 2>&5 ; then
      AC_MSG_RESULT([yes])
    else
      AC_MSG_RESULT([no])
      if test yes = "$with_python" ; then
	AC_MSG_ERROR([Couldn't import sphinx module and call sphinx.main() for Python2 - try package python-sphinx])
      fi
      PYTHON2=
    fi
    if test -n "$PYTHON2" ; then
      dnl Check that Python.h is there, which is a good way to check that
      dnl the appropriate python-dev package has been installed.
      AC_MSG_CHECKING([for $PYTHON2_INC/Python.h])
      if test -f "$PYTHON2_INC/Python.h" ; then
	AC_MSG_RESULT(yes)
	AC_MSG_CHECKING([for directory to install python2 bindings in])
	if test -z "$PYTHON2_LIB" ; then
	  PYTHON2_LIB=`$PYTHON2 -c 'import os,distutils.sysconfig;print(distutils.sysconfig.get_python_lib(1).replace(os.sep,"/"))'`
	fi
	AC_MSG_RESULT([$PYTHON2_LIB])
	AC_ARG_VAR(PYTHON2_LIB, [Directory to install python2 bindings in])

	AC_MSG_CHECKING([for python2 libraries to link against])
	case $host_os in
	mingw* | pw32*)
	  PYTHON2_LIBS=`$PYTHON2 -c 'import os,sys;print("-L"+os.path.join(sys.prefix,"libs").replace(os.sep,"/")+" -lpython"+sys.version[[:3]].replace(".",""))'` ;;
	cygwin*)
	  PYTHON2_LIBS=`$PYTHON2 -c 'import os,sys;print("-L"+os.path.join(sys.path[[3]],"config")+" -lpython"+sys.version[[:3]])'` ;;
	*)
	  PYTHON2_LIBS= ;;
	esac
	AC_SUBST(PYTHON2_LIBS)
	AC_MSG_RESULT([$PYTHON2_LIBS])

	AC_MSG_CHECKING([for python2 module extension])
	dnl Usually ".so", but for example, macOS uses ".dylib".
	PYTHON2_SO=`$PYTHON2 -c 'import distutils.sysconfig;print(distutils.sysconfig.get_config_vars("SO")[[0]])'`
	AC_SUBST(PYTHON2_SO)
	AC_MSG_RESULT([$PYTHON2_SO])
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

if test no != "$with_python3" ; then
  python3_ok=no
  dnl See comment for AC_PATH_PROGS(SWIG, ...).
  if test -n "$PYTHON3" ; then
    AC_PATH_PROGS(PYTHON3, ["$PYTHON3"], [])
  else
    AC_PATH_PROGS(PYTHON3, [python3 python], [])
  fi
  AC_ARG_VAR(PYTHON3, [Python 3 interpreter])
  if test -n "$PYTHON3" ; then
    python3_ok=yes
    dnl Require Python 3.3 or newer, as that's the oldest version we can
    dnl easily test with.  If anyone's keen to support older versions, please
    dnl test and send in any patches needed to get it to work.
    AC_MSG_CHECKING([$PYTHON3 version])
    version=`$PYTHON3 -c 'import sys;print("%d.%d" % sys.version_info[[:2]])' 2>/dev/null`
    case $version in
    [3.[3-9]*|3.[12][0-9]*|[4-9].*])
      AC_MSG_RESULT([$version])
      ;;
    *)
      AC_MSG_RESULT([$version (too old, only 3.3 or newer is supported)])
      python3_ok=no
      ;;
    esac
    if test yes = "$python3_ok" ; then
      PYTHON3_INC=`$PYTHON3 -c 'import os,sysconfig;print(sysconfig.get_path("include").replace(os.sep,"/"))' 2>/dev/null`
      AC_SUBST(PYTHON3_INC)

      AC_MSG_CHECKING([for python3 sphinx module])
      if $PYTHON3 -c 'import sphinx' >&5 2>&5 ; then
	AC_MSG_RESULT([yes])
      else
	AC_MSG_RESULT([no (try installing package python3-sphinx])
	python3_ok=no
      fi
    fi
    if test yes = "$python3_ok" ; then
      dnl Check that Python.h is there, which is a good way to check that
      dnl the appropriate python3-dev package has been installed.
      AC_MSG_CHECKING([for $PYTHON3_INC/Python.h])
      if test -f "$PYTHON3_INC/Python.h" ; then
	AC_MSG_RESULT(yes)
	AC_MSG_CHECKING([for directory to install python3 bindings in])
	if test -z "$PYTHON3_LIB" ; then
	  PYTHON3_LIB=`$PYTHON3 -c 'import os,sysconfig;print(sysconfig.get_path("platlib").replace(os.sep,"/"))'`
	fi
	AC_MSG_RESULT([$PYTHON3_LIB])
	AC_ARG_VAR(PYTHON3_LIB, [Directory to install python3 bindings in])

	AC_MSG_CHECKING([for python3 libraries to link against])
	case $host_os in
	mingw* | pw32*)
	  PYTHON3_LIBS=`$PYTHON3 -c 'import os,sys;print("-L"+os.path.join(sys.prefix,"libs").replace(os.sep,"/")+" -lpython"+sys.version[[:3]].replace(".",""))'` ;;
	cygwin*)
	  PYTHON3_LIBS=`$PYTHON3 -c 'import os,sys;print("-L"+os.path.join(sys.path[[3]],"config")+" -lpython"+sys.version[[:3]])'` ;;
	*)
	  PYTHON3_LIBS= ;;
	esac
	AC_SUBST(PYTHON3_LIBS)
	AC_MSG_RESULT([$PYTHON3_LIBS])

	AC_MSG_CHECKING([for python3 module extension])
	dnl Encodes the Python version the module is built for, and ends with
	dnl the platform-dependent shared object extension (which is `.so` on
	dnl most Unix-like platforms) - e.g. `.cpython-310-x86_64-linux-gnu.so`
	PYTHON3_EXT_SUFFIX=`$PYTHON3 -c 'import sysconfig;print(sysconfig.get_config_var("EXT_SUFFIX"))'`
	AC_SUBST(PYTHON3_EXT_SUFFIX)
	AC_MSG_RESULT([$PYTHON3_EXT_SUFFIX])

	AC_MSG_CHECKING([for tag for cached compiled scripts])
	PYTHON3_CACHE_TAG=`$PYTHON3 -c 'import sys;print(sys.implementation.cache_tag)'`
	AC_SUBST(PYTHON3_CACHE_TAG)
	AC_MSG_RESULT([$PYTHON3_CACHE_TAG])

	AC_MSG_CHECKING([for extension of cached and optimized python3 bytecode])
	dnl We use -O instead of cache_from_source()'s optimization parameter
	dnl as the latter was only added in Python 3.5.  The fallback is needed
	dnl as cache_from_source() itself is new in Python 3.4.
	dnl
	dnl Some Python versions (or maybe packages) need us to explicitly
	dnl `import importlib.util` here, but others work without it.
	dnl
	dnl This needs a shell-level || because Python one line program support
	dnl is poor.
	PYTHON3_CACHE_OPT1_EXT=`$PYTHON3 -O -c 'import importlib,importlib.util,sys;print("{1}{2}".format(*importlib.util.cache_from_source("").rpartition(sys.implementation.cache_tag)))' 2>/dev/null || $PYTHON3 -c 'import imp;print(imp.get_tag()+".pyo")'`
	AC_SUBST(PYTHON3_CACHE_OPT1_EXT)
	AC_MSG_RESULT([$PYTHON3_CACHE_OPT1_EXT])
      else
	AC_MSG_RESULT([no (install python3-dev or python3-devel package or similar)])
	python3_ok=no
      fi
    fi
  fi
  if test yes = "$python3_ok" ; then
    BINDINGS="$BINDINGS python3"
  elif test yes = "$with_python3" ; then
    AC_MSG_ERROR([Python3 bindings build dependencies not found])
  fi
fi

if test x$USE_MAINTAINER_MODE = xyes; then
  dnl Need python in maintainer mode to run doxy2swig.py.
  dnl Either Python 2 or 3 is OK for this.
  if test -n "$PYTHON2" ; then
    PYTHON=$PYTHON2
  elif test -n "$PYTHON3" ; then
    PYTHON=$PYTHON3
  else
    AC_PATH_PROGS(PYTHON, [python2 python python3], [])
  fi
  test -z "$PYTHON" && AC_MSG_ERROR([python is required in maintainer mode])
fi

if test no != "$with_php" ; then
  php_ok=no
  dnl See comment for AC_PATH_PROGS(SWIG, ...).
  if test -n "$PHP_CONFIG" ; then
    AC_PATH_PROGS(PHP_CONFIG, ["$PHP_CONFIG"], [])
  fi
  if test -z "$PHP_CONFIG" ; then
    AC_PATH_PROGS(PHP_CONFIG, [php-config8.4 php-config8.3 php-config8.2 php-config8.1 php-config8.0 php-config], [])
  fi
  AC_ARG_VAR(PHP_CONFIG, [php-config utility for PHP])
  AC_ARG_VAR(PHP, [PHP interpreter (optional - only needed to run PHP testsuite with)])
  if test -n "$PHP_CONFIG" ; then
    php_ok=yes
    AC_MSG_CHECKING([$PHP_CONFIG version])
    version=`$PHP_CONFIG --version 2>/dev/null`
    case $version in
    [[01234567].*])
      AC_MSG_RESULT([$version (not supported, PHP bindings need PHP >= 8)])
      php_ok=no
      ;;
    *)
      AC_MSG_RESULT([$version])
      PHP_MAJOR_VERSION=`echo "$version"|cut -d. -f1`
      ;;
    esac
    AC_SUBST(PHP_MAJOR_VERSION)

    if test yes = "$php_ok" ; then
      AC_MSG_CHECKING([for PHP extension directory])
      if test -z "$PHP_EXTENSION_DIR" ; then
	PHP_EXTENSION_DIR=`$PHP_CONFIG --extension-dir`
      fi
      AC_MSG_RESULT([$PHP_EXTENSION_DIR])
      AC_ARG_VAR(PHP_EXTENSION_DIR, [Directory to install PHP extensions in])

      PHP_INC=`$PHP_CONFIG --includes`
      AC_SUBST(PHP_INC)

      if test x"$PHP" = x ; then
	AC_MSG_CHECKING([for PHP interpreter])
	PHP=`$PHP_CONFIG --php-binary 2>/dev/null`
	if test x"$PHP" != x ; then
	  if $PHP -r 'exit(0);' 2> /dev/null ; then
	    if $PHP -r 'exit(PHP_MAJOR_VERSION>=8?0:1);' 2> /dev/null ; then
	      AC_MSG_RESULT([$PHP])
	    else
	      AC_MSG_RESULT([no ($PHP isn't PHP >= 8)])
	      PHP=
	    fi
	  else
	    AC_MSG_RESULT([no ($PHP does not work)])
	    PHP=
	  fi
	else
	  AC_MSG_RESULT([not found])
	fi
      else
	AC_PATH_PROGS(PHP, ["$PHP"], [])
      fi
      if test x"$PHP" = x ; then
	AC_MSG_WARN([No PHP interpreter found - PHP bindings tests will be skipped])
	dnl The skiptest script just returns exit code 77.
	PHP='$(top_srcdir)/skiptest'
      else
	AC_MSG_CHECKING([for PHP path separator])
	PHP_PATH_SEPARATOR=`$PHP -r 'echo PATH_SEPARATOR;'`
	AC_SUBST([PHP_PATH_SEPARATOR])
	AC_MSG_RESULT([$PHP_PATH_SEPARATOR])

	AC_MSG_CHECKING([for PHP shared library suffix])
	PHP_SHLIB_SUFFIX=`$PHP -r 'echo PHP_SHLIB_SUFFIX;'`
	AC_SUBST([PHP_SHLIB_SUFFIX])
	AC_MSG_RESULT([$PHP_SHLIB_SUFFIX])
      fi

      case $host_os in
      mingw* | pw32*)
	dnl This is a bit of an informed guess, pending more information from
	dnl someone who actually has mingw and wants to build the PHP bindings
	dnl there.  FIXME.
	PHP_LIBS="-L`$PHP_CONFIG --prefix` -lphp${PHP_MAJOR_VERSION}ts" ;;
      cygwin*)
	PHP_LIBS="-lphp${PHP_MAJOR_VERSION}" ;;
      *)
	PHP_LIBS= ;;
      esac
      AC_SUBST(PHP_LIBS)
    fi
  fi
  if test yes = "$php_ok" ; then
    BINDINGS="$BINDINGS php"
  elif test yes = "$with_php" ; then
    AC_MSG_ERROR([PHP bindings build dependencies not found])
  fi
fi

if test no != "$with_tcl" ; then
  tcl_ok=no
  dnl The documented minimum requirement is Tcl 8.5.  Older versions may work,
  dnl but we don't test with them regularly, and they are no longer supported
  dnl upstream.  Also ::tcl::pkgconfig was added in Tcl 8.5, which makes the
  dnl tests below simpler and more reliable.
  dnl
  dnl If you really want to build with an older version, run configure with
  dnl a lower tcl_min and specify at least TCL_INC explicitly:
  dnl
  dnl ./configure tcl_min=8.4 TCL_INC=/usr/include/tcl8.4
  dnl
  dnl We definitely need at least Tcl 8.1 for TCL_STUBS.
  : ${tcl_min=8.5}
  AC_PATH_PROGS(TCLSH, ["${TCLSH-tclsh}"], [])
  AC_ARG_VAR(TCLSH, [Tcl interpreter])
  if test -n "$TCLSH" ; then
    tcl_ok=yes
    AC_MSG_CHECKING([$TCLSH version])
    if echo 'if {$tcl_version < '"$tcl_min"' } { exit 1 }'|$TCLSH 2> /dev/null ; then
      tcl_version=`echo 'puts "$tcl_version"'|$TCLSH`
      AC_MSG_RESULT([$tcl_version])

      AC_ARG_VAR(TCL_LIB, [Directory to install Tcl files into])
      if test -z "$TCL_LIB" ; then
	[TCL_LIB=`echo 'foreach d [expr {[info exists tcl_pkgPath]?$tcl_pkgPath:$auto_path}] {if {![regexp {/share(/|$)} $d]} {break}}; puts $d'|$TCLSH`]
      fi
      AC_SUBST(TCL_LIB)

      AC_ARG_VAR([TCL_INC], [Directory to include for Tcl headers])
      if test -z "$TCL_INC" ; then
	dnl ::tcl::pkgconfig was added in Tcl 8.5.
	[TCL_INC=`echo 'puts [::tcl::pkgconfig get includedir,runtime]'|$TCLSH 2> /dev/null`]
      fi

      dnl Check that the headers are there (the tcl8.N-dev package
      dnl may not be installed).
      TCL_CPPFLAGS=
      AC_MSG_CHECKING([for tcl.h from Tcl $tcl_version])
      if test x"$TCL_INC" != x && test -f "$TCL_INC/tcl.h" ; then
	if test x"$TCL_INC" != x"/usr/include" ; then
	  TCL_CPPFLAGS="-I$TCL_INC"
	fi
	save_CPPFLAGS=$CPPFLAGS
	[tcl_major=`expr "$tcl_version" : '\([^.]*\)'`]
	[tcl_minor=`expr "$tcl_version" : '[^.]*\.\([^.]*\)'`]
	CPPFLAGS="$CPPFLAGS $TCL_CPPFLAGS -DMAJOR=$tcl_major -DMINOR=$tcl_minor"
	AC_COMPILE_IFELSE([AC_LANG_SOURCE([[
#include <tcl.h>
#if TCL_MAJOR_VERSION != MAJOR || TCL_MINOR_VERSION != MINOR
#error different tcl version
#endif
	  ]])], [
	    AC_MSG_RESULT([$TCL_INC/tcl.h])
	  ], [
	    AC_MSG_RESULT([$TCL_INC/tcl.h not from Tcl $tcl_version - specify TCL_INC on configure command line])
	    tcl_ok=no
	  ])
	CPPFLAGS=$save_CPPFLAGS
      else
	AC_MSG_RESULT([not found])
	tcl_ok=no
      fi

      TCL_LIBS=
      TCL_SHLIB_EXT=
      if test yes = "$tcl_ok" ; then
	dnl ::tcl::pkgconfig was added in Tcl 8.5.  Prior to 8.6 it's only
	dnl actually necessary to link against -ltclstub on platforms which
	dnl don't allow libraries to have unresolved symbols (cygwin, mingw,
	dnl msvc, etc)
	[tcl_libdir_runtime=`echo 'puts [::tcl::pkgconfig get libdir,runtime]'|$TCLSH 2> /dev/null`]
	if test -n "$tcl_libdir_runtime" ; then
	  for x in "/tcl$tcl_version" "" ; do
	    TCLCONFIG_SH=$tcl_libdir_runtime$x/tclConfig.sh
	    if test -r "$TCLCONFIG_SH" ; then
	      TCL_LIBS=`. $TCLCONFIG_SH && printf '%s' "$TCL_STUB_LIB_SPEC"`
	      break
	    fi
	  done
	fi
	TCL_CPPFLAGS="$TCL_CPPFLAGS -DUSE_TCL_STUBS"
	[TCL_SHLIB_EXT=`echo 'puts [info sharedlibextension]'|$TCLSH 2> /dev/null`]
      fi

      AC_SUBST(TCL_CPPFLAGS)
      AC_SUBST(TCL_LIBS)
      AC_SUBST(TCL_SHLIB_EXT)
    else
      AC_MSG_RESULT([< $tcl_min (too old)])
      tcl_ok=no
    fi
  fi
  if test yes = "$tcl_ok" ; then
    BINDINGS="$BINDINGS tcl8"
  elif test yes = "$with_tcl" ; then
    AC_MSG_ERROR([Tcl bindings build dependencies not found])
  fi
fi

if test no != "$with_csharp" ; then
  csharp_ok=no
  AC_ARG_VAR(CSC, [CSharp compiler command])
  if test -n "$CSC" ; then
    AC_PATH_PROGS(CSC, ["$CSC"], [])
  fi
  if test -z "$CSC" ; then
    dnl First we try cli-csc and mono-csc, which are used by Debian and Ubuntu
    dnl as "alternatives" which can be mapped to the CSharp compiler the admin
    dnl favours.
    AC_PATH_PROGS(CSC, [cli-csc mono-csc], [])
    if test -z "$CSC" ; then
      dnl mcs is the Mono CSharp compiler, which is a unified compiler
      dnl available from Mono 2.11 (and also the compiler in Mono 1.0, but
      dnl that's no longer relevant).
      AC_PATH_PROGS(CSC, [mcs], [])
      if test -n "$CSC" ; then
	dnl There are other tools called mcs (such as /usr/bin/mcs on Tru64),
	dnl so we check that the mcs we found understands --version which is
	dnl sufficient to distinguish mono's mcs from the Tru64 one.
	AC_MSG_CHECKING([whether $CSC is from GNU Mono])
	if (exec >&5 2>&5;$CSC --version </dev/null;exit $?) ; then
	  AC_MSG_RESULT(yes)
	  csharp_ok=yes
	else
	  AC_MSG_RESULT([no (ignoring $CSC)])
	fi
      fi
    fi
    if test no = "$csharp_ok" ; then
      dnl csc is the Microsoft CSharp compiler.
      AC_PATH_PROGS(CSC, [csc], [])
      if test -n "$CSC" ; then
	dnl Chicken (the Scheme-to-C compiler) includes a tool called csc so we
	dnl check if the output from "csc -version" includes the word chicken
	dnl which is sufficient to distinguish Chicken's csc from Microsoft's
	dnl csc.exe.
	AC_MSG_CHECKING([whether $CSC is for CSharp])
	if $CSC -version 2>/dev/null|grep chicken > /dev/null ; then
	  AC_MSG_RESULT([no (ignoring $CSC from Chicken)])
	else
	  AC_MSG_RESULT(yes)
	  csharp_ok=yes
	fi
      fi
    fi
  fi

  if test yes = "$csharp_ok" ; then
    AC_ARG_VAR(GACUTIL, [gacutil utility to use for CSharp bindings])
    if test -n "$GACUTIL" ; then
      AC_PATH_PROGS(GACUTIL, ["$GACUTIL"], [])
    fi
    if test -z "$GACUTIL" ; then
      AC_PATH_PROGS(GACUTIL, [cli-gacutil gacutil], [])
    fi

    AC_ARG_VAR(SN, [sn utility to use for CSharp bindings])
    if test -n "$SN" ; then
      AC_PATH_PROGS(SN, ["$SN"], [])
    fi
    if test -z "$SN" ; then
      AC_PATH_PROGS(SN, [cli-sn sn], [])
    fi
    if test -n "$GACUTIL" && test -n "$SN" ; then
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
	  AC_PATH_PROGS(CLI, ["${CLI-cli}"], [])
	  if test -z "$CLI" ; then
	    AC_PATH_PROGS(CLI, [mono ilrun], [])
	  fi
	  AC_ARG_VAR(CLI, [CSharp bytecode interpreter (optional - only needed to run CSharp testsuite with)])
	  if test -n "$CLI" ; then
	    AC_MSG_CHECKING([whether $CLI can run CSharp programs])
	    if test OK = "`$CLI ./conftest.exe 2> /dev/null`" ; then
	      AC_MSG_RESULT(yes)
	      RUN_CSHARP=$CLI
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

	AC_MSG_CHECKING([for extra flags for $GACUTIL])
	GACUTIL_FLAGS='/package $(ASSEMBLY) /gacdir $(libdir) /root $(DESTDIR)$(libdir)'
	dnl Microsoft's gacutil accepts fewer options than Mono's
	$GACUTIL /\?|grep '\<Mono\>' >/dev/null || GACUTIL_FLAGS=
	AC_SUBST([GACUTIL_FLAGS])
	if test -z "$GACUTIL_FLAGS" ; then
	  AC_MSG_RESULT([no])
	else
	  AC_MSG_RESULT([$GACUTIL_FLAGS])
	fi
      else
	AC_MSG_RESULT(no)
	csharp_ok=no
      fi
    else
      csharp_ok=no
    fi
  fi
  if test yes = "$csharp_ok" ; then
    BINDINGS="$BINDINGS csharp"
  elif test yes = "$with_csharp" ; then
    AC_MSG_ERROR([CSharp bindings build dependencies not found])
  fi
fi

JAVA_CPPFLAGS=
if test no != "$with_java" ; then
  java_ok=no
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
  if test -n "$JAVA" && test -n "$JAVAC" && test -n "$JAR" ; then
    dnl Eric says:
    dnl  The JNI library *requires* "Java 2", which is 1.2 or better.
    dnl
    dnl So checking for jni.h presumably implicitly checks we have "Java 2".
    dnl Note: jni.h #defines JNI_VERSION_1_[12468] (but not 3, 5 or 7 it seems).
    dnl So we could check for one of these if we want to check for a particular
    dnl JDK version...
    AC_CHECK_HEADER([jni.h], [java_ok=yes], [
      SAVE_CPPFLAGS=$CPPFLAGS
      real_javac=$JAVAC
      dnl Avoid endlessly traversing if there's a symlink loop.
      for count in 1 2 3 4 5 6 7 8 9 ; do
	r=`readlink "$real_javac"`
	test -n "$r" || break
	real_javac=$r
      done
      dnl Translate ".../bin/javac" to ".../include".
      [rel_jnidir=`echo "$real_javac"|sed 's!/[^/]*/[^/]*$!/include!'`]
      dnl Try the location for macOS system Java too, but last.
      for jnidir in \
	  $JNI_INCLUDE_DIR \
	  ${JAVA_HOME+"$JAVA_HOME/include"} \
	  ${JDK_HOME+"$JDK_HOME/include"} \
	  "$rel_jnidir" \
	  /System/Library/Frameworks/JavaVM.framework/Headers ; do
	CPPFLAGS="$SAVE_CPPFLAGS -I$jnidir"
	AC_MSG_CHECKING([for jni.h in $jnidir])
	if test -f "$jnidir/jni.h" ; then
	  AC_COMPILE_IFELSE([AC_LANG_SOURCE([[#include <jni.h>]])], [java_ok=yes])
	  if test yes = $java_ok ; then
	    AC_MSG_RESULT([yes])
	    JAVA_CPPFLAGS=-I$jnidir
	    break
	  fi

	  dnl For OpenJDK (and maybe others) we have to specify an extra -I
	  dnl option for an OS-specific subdirectory containing a "jni_md.h"
	  dnl header which is included by "jni.h".  Sadly we seem to need to
	  dnl hardcode a mapping for these platform names - the list below
	  dnl was determined by inspecting the source code of OpenJDK 8.
	  jni_host_os=
	  case $host_os in
	    linux*)
	      jni_host_os=linux ;;
	    solaris*)
	      jni_host_os=solaris ;;
	    mingw* | pw32* | cygwin*)
	      jni_host_os=win32 ;;
	    darwin*)
	      jni_host_os=darwin ;;
	    dragonfly*)
	      dnl Correct for Dragonfly openjdk17 package; also in `freebsd/`.
	      jni_host_os=dragonfly ;;
	    freebsd*)
	      dnl Correct for FreeBSD openjdk17 package.
	      jni_host_os=freebsd ;;
	    *bsd*)
	      jni_host_os=bsd ;;
	    aix*)
	      jni_host_os=aix ;;
	  esac
	  if test -n "$jni_host_os" ; then
	    CPPFLAGS="$CPPFLAGS -I$jnidir/$jni_host_os"
	    AC_COMPILE_IFELSE([AC_LANG_SOURCE([[#include <jni.h>]])], [java_ok=yes])
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

	  dnl If -Xcheck:jni is supported then run tests with it.
	  dnl
	  dnl ... except that with OpenJDK 8 and 9 -Xcheck:jni seems to report
	  dnl false positives and so can't usefully be turned on in this
	  dnl situation.  Recheck this with future versions.
dnl	  AC_MSG_CHECKING([for $JAVA -Xcheck:jni option])
dnl	  JAVA_PATHSEP2=`$JAVA conftest 2>&5`
	  JAVA_CHECK_JNI_OPTION=
dnl	  if test "$JAVA_PATHSEP" = "$JAVA_PATHSEP2" ; then
dnl	    JAVA_CHECK_JNI_OPTION=-Xcheck:jni
dnl	    AC_MSG_RESULT([yes])
dnl	  else
dnl	    AC_MSG_RESULT([no])
dnl	  fi
	  AC_SUBST([JAVA_CHECK_JNI_OPTION])
	else
	  AC_MSG_RESULT([failed to run Java test program])
	  java_ok=no
	fi
      else
	AC_MSG_RESULT([failed to compile Java test program])
	java_ok=no
      fi
    fi
  fi
  if test yes = "$java_ok" ; then
    BINDINGS="$BINDINGS java"
  elif test yes = "$with_java" ; then
    AC_MSG_ERROR([Java bindings build dependencies not found])
  fi
fi

if test no != "$with_ruby" ; then
  ruby_ok=no
  dnl See comment for AC_PATH_PROGS(SWIG, ...).
  if test -n "$RUBY" ; then
    AC_PATH_PROGS(RUBY, ["$RUBY"], [])
  else
    AC_PATH_PROGS(RUBY, [ruby], [])
  fi
  AC_ARG_VAR(RUBY, [Ruby interpreter])
  if test -n "$RUBY" ; then
    dnl Require Ruby 2.1 or newer.
    AC_MSG_CHECKING([$RUBY version])
    dnl RUBY_VERSION works with 1.6.  Once we've checked we have at least 1.8
    dnl we can safely use RbConfig below (RbConfig requires Ruby 1.8).
    version=`$RUBY -e 'print RUBY_VERSION' 2>/dev/null`
    case $version in
    "")
      AC_MSG_RESULT([\$RUBY -e 'print RUBY_VERSION' didn't work]) ;;
    [[01].*|2.0.*])
      AC_MSG_RESULT([$version (too old, only Ruby 2.1 or newer is supported)]) ;;
    *)
      ruby_ok=yes ;;
    esac
    if test yes = "$ruby_ok" ; then
      AC_MSG_RESULT([$version])
      AC_ARG_VAR(RUBY_INC, [Directory where ruby.h can be found])
      if test -z "$RUBY_INC" ; then
	dnl Ruby 1.9 added rubyhdrdir.
	[RUBY_INC=`$RUBY -rrbconfig -e 'print RbConfig::CONFIG["rubyhdrdir"]'`]
      fi
      AC_SUBST(RUBY_INC)
      dnl Check that ruby.h is there, which is a good way to check that
      dnl the appropriate ruby-dev package has been installed.
      AC_MSG_CHECKING([for $RUBY_INC/ruby.h])
      if test -f "$RUBY_INC/ruby.h" ; then
	AC_MSG_RESULT(yes)

	AC_ARG_VAR(RUBY_INC_ARCH, [Directory where ruby/config.h can be found])
	if test -z "$RUBY_INC_ARCH" ; then
	  dnl Ruby 2.0 and later have rubyarchhdrdir.
	  [RUBY_INC_ARCH=`$RUBY -rrbconfig -e 'print RbConfig::CONFIG["rubyarchhdrdir"]'`]
	fi
	AC_SUBST(RUBY_INC_ARCH)

	AC_ARG_VAR(RUBY_LIB, [Directory to install ruby files into])
	if test -z "$RUBY_LIB" ; then
	  [RUBY_LIB=`$RUBY -rrbconfig -e 'print RbConfig::CONFIG["sitelibdir"]'`]
	fi
	AC_SUBST(RUBY_LIB)

	AC_ARG_VAR(RUBY_LIB_ARCH, [Directory to install ruby binary modules into])
	if test -z "$RUBY_LIB_ARCH" ; then
	  [RUBY_LIB_ARCH=`$RUBY -rrbconfig -e 'print RbConfig::CONFIG["sitearchdir"]'`]
	fi
	AC_SUBST(RUBY_LIB_ARCH)

	RUBY_LIBS=
	if test unsupported = "$allow_undefined_flag" ; then
	  dnl For Microsoft Windows and a few other platforms.
	  [RUBY_LIBS=`$RUBY -rrbconfig -e 'print RbConfig::CONFIG["LIBRUBYARG"]'`]
	fi
	AC_SUBST(RUBY_LIBS)

	dnl rdoc is included as part of the Ruby core distribution, so run it
	dnl at build time rather shipping the rather large generated output in
	dnl our source tarball.
	if test -z "$RDOC" ; then
	  dnl Try replace the *last* 'ruby' with 'rdoc'.
	  rdoc_best_guess=`echo "$RUBY"|sed 's,\(.*\)ruby,\1rdoc,'`
	  if test -x "$rdoc_best_guess" ; then
	    RDOC=$rdoc_best_guess
	  fi
	fi
	AC_PATH_PROGS(RDOC, [rdoc], [])
	if test -z "$RDOC"; then
	  ruby_ok=no
	fi
	AC_ARG_VAR([RDOC], [Ruby documentation generator])

	RUBY_DLEXT=`$RUBY -rrbconfig -e 'print RbConfig::CONFIG[["DLEXT"]]'`
	AC_SUBST(RUBY_DLEXT)
      else
	AC_MSG_RESULT([no (install ruby-dev or ruby-devel package or similar)])
	ruby_ok=no
      fi
    fi
  fi
  if test yes = "$ruby_ok" ; then
    BINDINGS="$BINDINGS ruby"
  elif test yes = "$with_ruby" ; then
    AC_MSG_ERROR([Ruby bindings build dependencies not found])
  fi
fi

if test no != "$with_perl" ; then
  perl_ok=no
  AC_PATH_PROGS(PERL, ["${PERL-perl}"], [])
  AC_ARG_VAR(PERL, [Perl interpreter])
  if test -n "$PERL" ; then
    perl_ok=yes
    if test -z "$PERL_INC" ; then
      PERL_INC=`$PERL -MConfig -e 'print $Config{archlibexp}, "/CORE"'`
    fi
    AC_SUBST(PERL_INC)

    AC_ARG_VAR(PERL_ARCH, [Directory to install architecture-dependent perl files into])
    if test -z "$PERL_ARCH" ; then
      PERL_ARCH=`$PERL -MConfig -e 'print $Config{installsitearch}'`
    fi
    AC_SUBST(PERL_ARCH)

    AC_ARG_VAR(PERL_LIB, [Directory to install architecture-independent perl files into])
    if test -z "$PERL_LIB" ; then
      PERL_LIB=`$PERL -MConfig -e 'print $Config{installsitelib}'`
    fi
    AC_SUBST(PERL_LIB)

    AC_ARG_VAR(PERL_SO, [Extension for compiled Perl modules])
    if test -z "$PERL_SO" ; then
      PERL_SO=`$PERL -MConfig -e 'print ".", $Config{dlext}'`
    fi
    AC_SUBST(PERL_SO)

    if test -z "$PERL_XAPIAN_VERSION" ; then
      dnl snapshot versions look like 1.2.3_git123
      case $PACKAGE_VERSION in
      *_git*)
	PERL_XAPIAN_VERSION=`echo "$PACKAGE_VERSION"|sed 's/_git/./'` ;;
      *)
	PERL_XAPIAN_VERSION=$PACKAGE_VERSION.0 ;;
      esac
    fi
    AC_SUBST(PERL_XAPIAN_VERSION)
  fi
  if test yes = "$perl_ok" ; then
    BINDINGS="$BINDINGS perl"
  elif test yes = "$with_perl" ; then
    AC_MSG_ERROR([Perl bindings build dependencies not found])
  fi
fi

if test no != "$with_lua" ; then
  lua_ok=no
  if test -n "$LUA" ; then
    AC_PATH_PROGS(LUA, ["$LUA"], [])
  fi
  if test -z "$LUA" ; then
    AC_PATH_PROGS(LUA, [lua lua5.4 lua54 lua5.3 lua53 lua5.2 lua52 lua5.1 lua51], [])
  fi
  AC_ARG_VAR(LUA, [lua interpreter])
  if test -n "$LUA" ; then
    lua_ok=yes
    AC_MSG_CHECKING([$LUA version])
    lua_version=`$LUA -e 'print(_VERSION:sub(_VERSION:find(" ")+1))' 2>&AS_MESSAGE_LOG_FD`
    if test -n "$lua_version"; then
      AC_MSG_RESULT([$lua_version])
      dnl We need Lua 5.1 or later.  Unhelpfully there are different naming
      dnl schemes for Lua in pkg-config on different platforms.
      PKG_CHECK_MODULES([LUA], [lua$lua_version >= 5.1], [], [
	PKG_CHECK_MODULES([LUA], [lua-$lua_version >= 5.1], [], [
	  if test yes = "$with_lua" ; then
	    AC_MSG_ERROR([pkg-config module for LUA $lua_version not found])
	  fi
	  lua_ok=no
	])
      ])
    else
      AC_MSG_RESULT([could not determine])
    fi

    if test yes = "$lua_ok" ; then
      AC_ARG_VAR([LUA_LIB], [Directory to install compiled Lua modules into])
      AC_ARG_VAR([LUA_SO], [Extension for compiled Lua modules (e.g. LUA_SO=.so)])
      if test -z "$LUA_LIB" || test -z "$LUA_SO" ; then
	dnl If LUA_SO is specified, ensure it has a leading `.`.
	case $LUA_SO in
	  "") ;;
	  .*) ;;
	  *) LUA_SO=.$LUA_SO ;;
	esac
	dnl Look for the first absolute path in lua_cpath which has leaf
	dnl "?.<ext>" where <ext> is not "lua".  If LUA_SO was specified by the
	dnl user, only consider entries where <ext> matches LUA_SO.
	lua_lib_and_so=`$LUA "$srcdir/lua/unpick-cpath.lua" "$LUA_SO"`
	test -n "$LUA_LIB" || LUA_LIB=`expr X"$lua_lib_and_so" : X'\(.*\);'`
	test -n "$LUA_SO" || LUA_SO=`expr X"$lua_lib_and_so" : '.*;\(.*\)'`
      fi
      AC_MSG_CHECKING([for directory to install compiled Lua modules into])
      if test -z "$LUA_LIB"; then
	AC_MSG_RESULT([not found])
	lua_ok=no
      else
	AC_MSG_RESULT([$LUA_LIB])
      fi
      AC_SUBST([LUA_LIB])

      AC_MSG_CHECKING([for extension to use for compiled Lua modules])
      if test -z "$LUA_SO"; then
	AC_MSG_RESULT([not found])
	lua_ok=no
      else
	AC_MSG_RESULT([$LUA_SO])
      fi
      AC_SUBST([LUA_SO])
    fi
  fi

  if test yes = "$lua_ok" ; then
    BINDINGS="$BINDINGS lua"
  elif test yes = "$with_lua" ; then
    AC_MSG_ERROR([lua not found])
  fi
fi

AC_SUBST(BINDINGS)

case $BINDINGS in
*python*) dnl Matches python and/or python3
  dnl C++11 has thread_local
  dnl GCC and some others have __thread
  dnl MSVC and some others have __declspec(thread)
  AC_MSG_CHECKING([for thread-local storage qualifier])
  AC_ARG_VAR([THREAD_LOCAL], [thread-local storage qualifier])
  if test -z "$THREAD_LOCAL" ; then
    for t in thread_local __thread '__declspec(thread)' ; do
      AC_COMPILE_IFELSE([AC_LANG_SOURCE([[static $t void * p;]])],
			[THREAD_LOCAL=$t; break])
    done
  fi
  if test -z "$THREAD_LOCAL" ; then
    AC_MSG_RESULT([unknown, disabling thread-local support])
  else
    AC_MSG_RESULT([$THREAD_LOCAL])
    AC_DEFINE_UNQUOTED([THREAD_LOCAL], [$THREAD_LOCAL], [Thread local storage qualifier (undefined if no TLS)])
  fi
  ;;
esac

dnl Set flags to control warnings (enable more, or disable annoying ones)
dnl and other compiler specific flags.
SWIG_CXXFLAGS=
if test yes = "$GXX" ; then
  dnl Python itself is compiled with -fno-strict-aliasing, and it appears
  dnl it's safest to follow this lead when compiling the SWIG generated
  dnl interface code.  E.g.:
  dnl   https://article.gmane.org/gmane.comp.gcc.devel/74692
  dnl The code SWIG generates for other languages seems to have similar
  dnl issues too, so just turn this on for all of them.
  dnl
  dnl There's no need to check -fno-strict-aliasing is supported as it works
  dnl with GCC 2.95 and we don't support anything older.
  SWIG_CXXFLAGS="-fno-strict-aliasing"

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
    ],
    [
      dnl GCC or clang:
      dnl
      dnl Swig generated code gives lots of unused and uninitialized warnings.
      dnl They're non-harmful, so suppress them.
      SWIG_CXXFLAGS="$SWIG_CXXFLAGS -Wall -Wno-unused -Wno-uninitialized"
    ])

  if test no != "$enable_visibility"; then
    dnl GCC doesn't support symbol visibility on all platforms (e.g. it isn't
    dnl supported on mingw and visibility attributes result in warnings).
    AC_MSG_CHECKING([if $CXX -fvisibility=hidden -fvisibility-inlines-hidden works])
    if echo 'int bar() __attribute__((visibility("default"))); int baz() __attribute__((visibility("internal"))); int bar() { return 6; } int baz() { return 7; } int foo() {return 42;}'|$CXX -Werror -fvisibility=hidden -fvisibility-inlines-hidden -c -oconftest.o -xc++ - >&AS_MESSAGE_LOG_FD 2>&AS_MESSAGE_LOG_FD ; then
      AC_MSG_RESULT([yes])
      SWIG_CXXFLAGS="$SWIG_CXXFLAGS -fvisibility=hidden -fvisibility-inlines-hidden"
    else
      AC_MSG_RESULT([no])
    fi
  fi
fi
AC_SUBST(SWIG_CXXFLAGS)

dnl Restore CXXFLAGS to those the user specified or autoconf defaulted to.
CXXFLAGS=$save_CXXFLAGS

dnl Required for auto regeneration to work in a combined maintainer-mode tree.
: ${AUTOM4TE=autom4te}
AC_SUBST([AUTOM4TE])

AC_CONFIG_FILES([Makefile
 doxygen_xml.conf
 xapian-version.h
 python/Makefile python/docs/conf.py python/version.i
 python3/Makefile python3/docs/conf.py python3/version.i
 php/Makefile
 java/Makefile
 tcl8/Makefile tcl8/pkgIndex.tcl
 csharp/Makefile csharp/AssemblyInfo.cs
 ruby/Makefile
 perl/Makefile
 lua/Makefile
 xapian-bindings.spec
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
  release_series=[`echo "$COMPAT_VERSION"|sed 's/[0-9][0-9]*$//'`]
  case $XAPIAN_VERSION in
  $release_series*) ;;
  *)
    AC_MSG_ERROR([Xapian library is version ${XAPIAN_VERSION-<0.8.2} which is from a different release series to bindings $PACKAGE_VERSION, so they aren't compatible.])
    ;;
  esac
  dnl 0.8.2 was the first version which set XAPIAN_VERSION
  AC_MSG_WARN([Xapian library is version ${XAPIAN_VERSION-<0.8.2} but the bindings are version $PACKAGE_VERSION - we strongly recommend using matching versions.])
fi
