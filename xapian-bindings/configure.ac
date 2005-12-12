dnl Process this file with autoconf to produce a configure script.

dnl Need autoconf 2.50 or later for many features
dnl 2.54 needed for automake 1.6
dnl 2.57 fixes the annoying warnings from configure on FreeBSD
dnl 2.58 is needed by automake 1.8.5
dnl 2.59 was released the same day as 2.58 to fix a problem
AC_PREREQ(2.59)
AC_INIT(xapian-bindings, 0.9.2)dnl FIXME: bugreport addr as third argument
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR(xapian.i)

AM_CONFIG_HEADER(config.h)

dnl MacOS X needs MACOSX_DEPLOYMENT_TARGET set to 10.3 (or higher) to
dnl successfully link the SWIG generated modules.  The link always seems
dnl to fail on 10.1.4 whatever I try.  Not tried 10.2.8 yet as that SF CF box
dnl is down but I expect it will fail there too.  So always setting
dnl MACOSX_DEPLOYMENT_TARGET to at least 10.3 seems to be the appropriate
dnl thing to do.  Most MacOS X developers will have upgraded from 10.1 and
dnl 10.2 by now anyhow.
dnl
dnl NB we must do this *BEFORE* AC_PROG_LIBTOOL since that is what checks
dnl MACOSX_DEPLOYMENT_TARGET.
AC_CANONICAL_HOST
case $host_os in
darwin*)
  case $MACOSX_DEPLOYMENT_TARGET in
  "")
    MACOSX_DEPLOYMENT_TARGET=10.3 ;;
  10.[[012]])
    AC_MSG_WARN([Overriding MACOSX_DEPLOYMENT_TARGET from $MACOSX_DEPLOYMENT_TARGET to 10.3])
    MACOSX_DEPLOYMENT_TARGET=10.3 ;;
  esac
esac

dnl Use libtool to manage our libraries, but don't build static libraries as
dnl the bindings only have a use for dynamic ones.
AC_DISABLE_STATIC
AC_PROG_LIBTOOL

dnl Checks for programs.
AC_PROG_CXX

dnl Run tests using the C++ compiler.
AC_LANG_CPLUSPLUS

dnl Check for libxapian
XO_LIB_XAPIAN(, AC_MSG_ERROR([Can't find Xapian library]))

dnl We want XAPIAN_CXXFLAGS to be used for configure tests, so set AM_CXXFLAGS
dnl here and unset them later.
AM_CXXFLAGS="$XAPIAN_CXXFLAGS"

dnl Allow for bindings being version 0.8.5.1 which xapian-core is 0.8.5.
dnl Also ignore any _svn6789 suffix which snapshots will have.
COMPAT_VERSION=[`echo "$PACKAGE_VERSION"|sed 's/^\([0-9]*\.[0-9]*\.[0-9]*\).*/\1/'`]
if test "$COMPAT_VERSION" != "$XAPIAN_VERSION" ; then
  dnl 0.8.2 was the first version which set XAPIAN_VERSION
  AC_MSG_ERROR([Xapian library is version ${XAPIAN_VERSION-<0.8.2} but the bindings are version $PACKAGE_VERSION])
fi

dnl Only probe for SWIG and enable SWIG rules in makefiles if
dnl configure --enable-maintainer-mode is used.
AM_MAINTAINER_MODE

if test x$USE_MAINTAINER_MODE = xyes; then
  dnl Check for swig - this does most of the work for the bindings.
  AC_PATH_PROG(SWIG, swig, [])
  if test -z "$SWIG" ; then
    AC_MSG_ERROR([Can't find SWIG utility])
  fi
  dnl Check for swig >= 1.3.26
  dnl We need 1.3.14 for overloaded functions.
  dnl 1.3.19 was missing std_common.i - it was empty at the time so you
  dnl can just touch it into existence, but it's cleaner to just require
  dnl 1.3.20.
  dnl 1.3.20 is required for -python -modern.
  dnl 1.3.21 fixes a few minor issues which don't look to be relevant to
  dnl us currently.
  dnl 1.3.22 fixes a problem with wrapping classes in namespaces with guile.
  dnl 1.3.23 adds support for optional arguments in C#.
  dnl 1.3.24 tweaks C# enums
  dnl 1.3.26 fixes PHP overloading
  dnl 1.3.27 contains a few fixes, none of which affect us
  dnl You can see the latest SWIG changes here:
  dnl http://cvs.sourceforge.net/viewcvs.py/*checkout*/swig/SWIG/CHANGES
  v=`$SWIG -version 2>&1|sed 's/^SWIG Version \([[0-9\.]]*\).*/\1/p;d'`
  case $v in
   [1.3.2[6-9]|1.3.[3-9][0-9]|1.3.[1-9][0-9][0-9][0-9]*|1.[4-9].*|2.*]) ;;
   *)
     AC_MSG_ERROR([SWIG >= 1.3.26 required (you have $v)]) ;;
  esac
  SWIG_FLAGS="`$XAPIAN_CONFIG --swigflags` -Werror"
  AC_SUBST(SWIG_FLAGS)
fi

AC_ARG_WITH(swig,
  AC_HELP_STRING([--with-swig],
		 [enable all bindings which depend on SWIG (currently: Python PHP Tcl Guile CSharp)]),
  [],
  [with_swig=])

AC_ARG_WITH(python,
  AC_HELP_STRING([--with-python], [enable Python bindings]),
  [],
  [with_python=$with_swig])

AC_ARG_WITH(php,
  AC_HELP_STRING([--with-php], [enable PHP bindings]),
  [],
  [with_php=$with_swig])

AC_ARG_WITH(tcl,
  AC_HELP_STRING([--with-tcl], [enable Tcl bindings]),
  [],
  [with_tcl=$with_swig])

AC_ARG_WITH(guile,
  AC_HELP_STRING([--with-guile], [enable Guile bindings]),
  [],
  [with_guile=$with_swig])

AC_ARG_WITH(csharp,
  AC_HELP_STRING([--with-csharp], [enable CSharp bindings]),
  [],
  [with_csharp=$with_swig])

AC_ARG_WITH(java,
  AC_HELP_STRING([--with-java], [enable Java bindings]),
  [],
  [with_java=])

case $with_swig$with_python$with_php$with_tcl$with_guile$with_csharp$with_java in
*yes*)
  dnl Default unspecified values to no.
  test -z "$with_python" && with_python=no
  test -z "$with_php" && with_php=no
  test -z "$with_tcl" && with_tcl=no
  test -z "$with_guile" && with_guile=no
  test -z "$with_csharp" && with_csharp=no
  test -z "$with_java" && with_java=no
  ;;
esac

BINDINGS=

if test no != "$with_python" ; then
  AC_PATH_PROG(PYTHON, python, [])
  if test -n "$PYTHON" ; then
    dnl Require Python 2.1 or newer.  James says:
    dnl  2.0 as a minimum. I'm using __cmp__ / __eq__ tricks to get the
    dnl  iterators to behave like Python iterators, so not all aspects will
    dnl  work without 2.1; I'd be happy to require 2.1 though, because it's
    dnl  been out for a long time, and 2.0 wasn't around for all that long as
    dnl  I remember.
    version=`$PYTHON -c 'import sys;print sys.version[[:3]]' 2>/dev/null`
    case $version in
    2.1)
      PYTHON_MODERN_OR_OLDE=olde
      ;;
    [2.[1-9]*|[3-9].*])
      dnl Used output from swig -modern for Python 2.2 and up for
      dnl cleaner, leaner, faster wrapper code in xapian.py.
      PYTHON_MODERN_OR_OLDE=modern
      ;;
    *)
      if test yes = "$with_python" ; then
	AC_MSG_ERROR([Only python 2.1 or newer is supported ($python is $version)])
      fi
      PYTHON=
      ;;
    esac
    if test -n "$PYTHON" ; then
      AC_SUBST(PYTHON_MODERN_OR_OLDE)
      PYTHON_INC=`$PYTHON -c 'import os,distutils.sysconfig;print distutils.sysconfig.get_python_inc().replace(os.sep,"/")'`
      AC_SUBST(PYTHON_INC)
      dnl Check that Python.h is there, which is a good way to check that
      dnl the appropriate python-dev package has been installed.
      AC_MSG_CHECKING([for $PYTHON_INC/Python.h])
      if test -f "$PYTHON_INC/Python.h" ; then
	AC_MSG_RESULT(yes)
	PYTHON_LIB=`$PYTHON -c 'import os,distutils.sysconfig;print distutils.sysconfig.get_python_lib(1).replace(os.sep,"/")'`
	AC_ARG_VAR(PYTHON_LIB, [Directory to install python bindings in])
	PYTHON_PATHSEP=`python -c 'import os;print os.pathsep'`
	AC_SUBST(PYTHON_PATHSEP)
	case $host_os in
	mingw* | pw32*)
	  PYTHON_LIBS=`$PYTHON -c 'import os,sys;print "-L"+os.path.join(sys.prefix,"libs").replace(os.sep,"/"),"-lpython"+sys.version[[:3]].replace(".","")'` ;;
	*)
	  PYTHON_LIBS=
	esac
	AC_SUBST(PYTHON_LIBS)
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
  AC_PATH_PROG(PHP_CONFIG, [php-config], [])
  if test -n "$PHP_CONFIG" ; then
    version=`$PHP_CONFIG --version 2> /dev/null`
    case $version in
    4.*)
      dnl If the php interpreter can't be found, skip the php tests.
      AC_PATH_PROGS(PHP, [php4 php], [\$(top_srcdir)/skiptest])
      PHP_INC=`$PHP_CONFIG --includes`
      AC_SUBST(PHP_INC)
      PHP_EXTENSION_DIR=`$PHP_CONFIG --extension-dir`
      AC_SUBST(PHP_EXTENSION_DIR)
      BINDINGS="$BINDINGS php"
      ;;
    5.*)
      dnl If the php interpreter can't be found, skip the php tests.
      AC_PATH_PROGS(PHP, [php5 php], [\$(top_srcdir)/skiptest])
      PHP_INC=`$PHP_CONFIG --includes`
      AC_SUBST(PHP_INC)
      PHP_EXTENSION_DIR=`$PHP_CONFIG --extension-dir`
      AC_SUBST(PHP_EXTENSION_DIR)
      BINDINGS="$BINDINGS php"
      ;;
    *)
      if test yes = "$with_php" ; then
	AC_MSG_ERROR([Only PHP4 and PHP5 are supported ($PHP_CONFIG reports $version)])
      fi
      ;;
    esac
  fi
fi

if test no != "$with_tcl" ; then
  AC_PATH_PROG(TCLSH, tclsh, [])
  if test -n "$TCLSH" ;  then
    # We need Tcl 8.1 or later for TCL_STUBS.
    # Note: The bindings can easily be made to work with Tcl 8.0 (just
    # don't pass -DUSE_TCL_STUBS when compiling) should you need that.
    if `echo 'if {$tcl_version < 8.1 } { exit 1 }'|$TCLSH 2> /dev/null` ; then
      TCL_LIB=`echo 'puts $tcl_library'|$TCLSH`
      AC_SUBST(TCL_LIB)
      dnl There's no obvious way to get "/usr/include/tcl8.N" directly...
      dnl debian uses /usr/lib/tcl8.4, FC2 uses /usr/share/tcl8.4
      TCL_INC=`echo "$TCL_LIB"|sed 's!/\(share\|lib\)/!/include/!'`
      dnl Check that the headers are there (the tcl8.N-dev package
      dnl may not be installed).
      TCL_CPPFLAGS=
      if test -f "$TCL_INC/tcl.h" ; then
	BINDINGS="$BINDINGS tcl8"
	TCL_CPPFLAGS="-I$TCL_INC"
      elif test -f "/usr/include/tcl.h" ; then
	if test x`awk '($1 == "#define" && $2 == "TCL_VERSION") {print $3}' /usr/include/tcl.h` = x'"'`echo 'puts $tcl_version'|$TCLSH`'"' ; then
	  BINDINGS="$BINDINGS tcl8"
	else
	  if test yes = "$with_tcl" ; then
	    AC_MSG_ERROR([Can't find tcl.h for version $TCL_VERSION])
	  fi
	fi
      else
	if test yes = "$with_tcl" ; then
	  AC_MSG_ERROR([Can't find tcl.h])
	fi
      fi
      AC_SUBST(TCL_CPPFLAGS)
    fi
  else
    if test yes = "$with_tcl" ; then
      AC_MSG_ERROR([tclsh not found or too old (Tcl 8.1 or newer required)])
    fi
  fi
fi

if test yes == "$with_guile" ; then
  AC_PATH_PROG(GUILE, guile, [])
  if test -n "$GUILE" ;  then
    dnl test guile version?
    dnl any paths?
    BINDINGS="$BINDINGS guile"
  else
    if test yes = "$with_guile" ; then
      AC_MSG_ERROR([guile not found])
    fi
  fi
fi

if test no != "$with_csharp" ; then
  AC_PATH_PROG(CSC, mcs, [])
  if test -n "$CSC" ; then
    # There are other tools called mcs (such as /usr/bin/mcs on Tru64),
    # so we check that the mcs we found understands --version which is
    # sufficient to distinguish mono's mcs from the Tru64 one.
    AC_MSG_CHECKING([whether $CSC is from GNU Mono])
    if (exec >&5 2>&5;$CSC --version </dev/null;exit $?) ; then
      AC_MSG_RESULT(yes)
      AC_PATH_PROG(GACUTIL, gacutil, [])
      AC_PATH_PROG(SN, sn, [])
      if test -n "$GACUTIL" -a -n "$SN" ; then
	BINDINGS="$BINDINGS csharp"
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
      AC_MSG_RESULT(no)
      if test yes = "$with_csharp" ; then
	AC_MSG_ERROR([Mono mcs not found (found different mcs program)])
      fi
      CSC=
    fi
  else
    if test yes = "$with_csharp" ; then
      AC_MSG_ERROR([Mono mcs not found])
    fi
  fi
fi

if test no != "$with_java" ; then
  AC_PATH_PROG(JAVA, java, [],
  	[${JAVA_HOME+$JAVA_HOME/bin:}${JDK_HOME+$JDK_HOME/bin:}$PATH])
  AC_PATH_PROG(JAVAC, javac, [],
  	[${JAVA_HOME+$JAVA_HOME/bin:}${JDK_HOME+$JDK_HOME/bin:}$PATH])
  AC_PATH_PROG(JAR, jar, [],
  	[${JAVA_HOME+$JAVA_HOME/bin:}${JDK_HOME+$JDK_HOME/bin:}$PATH])
  if test -n "$JAVA" -a -n "$JAVAC" -a -n "$JAR" ; then
    dnl Eric says:
    dnl  The JNI library *requires* "Java 2", which is 1.2 or better.
    dnl  I've only tested with JDK 1.3 and 1.4 from Sun.
    dnl
    dnl So checking for jni.h presumably checks we have "Java 2"?!
    dnl Note: jni.h #defines JNI_VERSION_1_[124] (but not 3 it seems).
    dnl So we could check for that if we want to check for a particular
    dnl JDK version...
    java_ok=no
    AC_CHECK_HEADER(jni.h, [java_ok=yes],
      [test yes = "$with_java" && AC_MSG_ERROR([jni.h not found])])
    if test yes = $java_ok ; then
      AC_MSG_CHECKING([for Java path separator])
      [echo 'public class conftest { public static void main(String[] args) { System.out.println(System.getProperty("path.separator")); } }' > conftest.java]
      if (exec >&5 2>&5;$JAVAC conftest.java;exit $?) ; then
	JAVA_PATHSEP=`$JAVA conftest 2> /dev/null`
	AC_SUBST(JAVA_PATHSEP)
	if test -n "$JAVA_PATHSEP" ; then
	  AC_MSG_RESULT($JAVA_PATHSEP)
	  BINDINGS="$BINDINGS java"
	else
	  java_ok=no
	fi
      else
	java_ok=no
      fi
      if test no = "$java_ok" ; then
	AC_MSG_RESULT([test failed])
        test yes = "$with_java" && AC_MSG_ERROR([Couldn't compile and run simple Java program])
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

AC_SUBST(BINDINGS)

dnl extra flags for building with gcc
SWIG_CXXFLAGS=
JAVA_CXXFLAGS=
if test yes = "$GXX" ; then
  dnl Swig generated code gives lots of unused and uninitialized warnings.
  dnl They're non-harmful, so suppress them.
  SWIG_CXXFLAGS="-Wall -Wno-unused -Wno-uninitialized"
  JAVA_CXXFLAGS="-Wall -W -Wno-unused"
  dnl We need to explicitly link against -lstdc++ on OpenBSD (discovered
  dnl on OpenBSD 3.7 with GCC 3.3.5 but this appears to be due to a
  dnl deliberate decision on the part of OpenBSD developers).  Luckily
  dnl we can just always specify -lstdc++ explicitly if GCC is the
  dnl compiler and libtool will eliminate the duplicate on other
  dnl platforms.
  XAPIAN_LIBS="$XAPIAN_LIBS -lstdc++"
fi
AC_SUBST(SWIG_CXXFLAGS)
AC_SUBST(JAVA_CXXFLAGS)

AM_CXXFLAGS=

AC_CONFIG_FILES([Makefile
 python/Makefile python/docs/Makefile python/docs/examples/Makefile
 php/Makefile php/docs/Makefile php/docs/examples/Makefile php/util.i
 java/Makefile java/native/Makefile java/org/Makefile java/org/xapian/Makefile
 java/org/xapian/errors/Makefile java/org/xapian/examples/Makefile
 guile/Makefile
 tcl8/Makefile tcl8/docs/Makefile tcl8/docs/examples/Makefile
 csharp/Makefile
 csharp/AssemblyInfo.cs
 xapian-bindings.spec])
AC_OUTPUT
