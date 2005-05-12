dnl Process this file with autoconf to produce a configure script.

dnl Need autoconf 2.50 or later for many features
dnl 2.54 needed for automake 1.6
dnl 2.57 fixes the annoying warnings from configure on FreeBSD
dnl 2.58 is needed by automake 1.8.5
dnl 2.59 was released the same day as 2.58 to fix a problem
AC_PREREQ(2.59)
AC_INIT(xapian-bindings, 0.9.0)dnl FIXME: bugreport addr as third argument
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR(xapian.i)

AM_CONFIG_HEADER(config.h)

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
dnl We want XAPIAN_CXXFLAGS to be used for configure tests
CXXFLAGS="$CXXFLAGS $XAPIAN_CXXFLAGS"

dnl Allow for bindings being version 0.8.5.1 which xapian-core is 0.8.5.
dnl Also ignore any _svn6789 suffix which snapshots will have.
COMPAT_VERSION=["`echo \"$PACKAGE_VERSION\"|sed 's/^\([0-9]*\.[0-9]*\.[0-9]*\).*/\1/'`"]
if test "$COMPAT_VERSION" != "$XAPIAN_VERSION" ; then
    dnl 0.8.2 was the first version which set XAPIAN_VERSION
    AC_MSG_ERROR([Xapian library is version ${XAPIAN_VERSION-<0.8.2} but the bindings are version $PACKAGE_VERSION])
fi

SWIG_FLAGS="`$XAPIAN_CONFIG --swigflags` -Werror"
AC_SUBST(SWIG_FLAGS)

AC_ARG_WITH(swig,
  [  --without-swig          disable all bindings which depend on SWIG
                         (currently: Python PHP Tcl Guile CSharp)])
AC_ARG_WITH(python,
  [  --without-python        disable Python bindings])

AC_ARG_WITH(php,
  [  --without-php           disable PHP bindings])

AC_ARG_WITH(tcl,
  [  --without-tcl           disable Tcl bindings])

AC_ARG_WITH(java,
  [  --without-java          disable Java bindings])

AC_ARG_WITH(guile,
  [  --without-guile         disable Guile bindings])

AC_ARG_WITH(csharp,
  [  --without-csharp        disable CSharp bindings])

BINDINGS=

dnl Only probe for SWIG and enable SWIG rules in makefiles if
dnl configure --enable-maintainer-mode is used.
AM_MAINTAINER_MODE

if test x$USE_MAINTAINER_MODE = xyes; then
  dnl Check for swig - this does most of the work for the bindings.
  AC_PATH_PROG(SWIG, swig, [])
  if test -z "$SWIG" ; then
    AC_MSG_ERROR([Can't find SWIG utility])
  fi
  dnl Check for swig >= 1.3.24
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
  dnl You can see the latest SWIG changes here:
  dnl http://cvs.sourceforge.net/viewcvs.py/*checkout*/swig/SWIG/CHANGES
  v="`$SWIG -version 2>&1|sed 's/^SWIG Version \([[0-9\.]]*\).*/\1/p;d'`"
  case "$v" in
   [1.3.2[4-9]|1.3.[3-9][0-9]|1.3.[1-9][0-9][0-9][0-9]*|1.[4-9].*|2.*]) ;;
   *)
     AC_MSG_ERROR([SWIG >= 1.3.24 required (you have $v)]) ;;
  esac
  AC_PATH_PROG(SWIG_1_3_22, [swig-1.3.22], [])
  if test -z "$SWIG_1_3_22" ; then
    AC_MSG_ERROR([Can't find SWIG utility version 1.3.22])
  fi
  dnl SWIG 1.3.23 changes how overloading works, and this causes problems with
  dnl optional arguments in PHP, so we need to stick to 1.3.22 for PHP for now.
  v="`$SWIG_1_3_22 -version 2>&1|sed 's/^SWIG Version \([[0-9\.]]*\).*/\1/p;d'`"
  if test 1.3.22 != "$v" ; then
    AC_MSG_ERROR([\$SWIG_1_3_22 ($SWIG_1_3_22) isn't SWIG 1.3.22 (it's $v)])
  fi
fi

if test no != "$with_swig" ; then
  if test no != "$with_python" ; then
    AC_PATH_PROG(PYTHON, python, [])
    if test -n "$PYTHON" ; then
      dnl Require Python 2.1 or newer.  James says:
      dnl  2.0 as a minimum. I'm using __cmp__ / __eq__ tricks to get the
      dnl  iterators to behave like Python iterators, so not all aspects will
      dnl  work without 2.1; I'd be happy to require 2.1 though, because it's
      dnl  been out for a long time, and 2.0 wasn't around for all that long as
      dnl  I remember.
      case "`$PYTHON -c 'import sys;print sys.version[[:3]]' 2>/dev/null`" in
      2.1)
	PYTHON_MODERN_OR_OLDE=olde
	;;
      [2.[1-9]*|[3-9].*])
	dnl Used output from swig -modern for Python 2.2 and up for
	dnl cleaner, leaner, faster wrapper code in xapian.py.
	PYTHON_MODERN_OR_OLDE=modern
	;;
      *)
	PYTHON=
	;;
      esac
      if test -n "$PYTHON" ; then
	AC_SUBST(PYTHON_MODERN_OR_OLDE)
	PYTHON_INC="`$PYTHON -c 'import os,distutils.sysconfig;print distutils.sysconfig.get_python_inc().replace(os.sep,"/")'`"
	AC_SUBST(PYTHON_INC)
	dnl Check that Python.h is there, which is a good way to check that
	dnl the appropriate python-dev package has been installed.
	AC_MSG_CHECKING([for $PYTHON_INC/Python.h])
	if test -f "$PYTHON_INC/Python.h" ; then
	  AC_MSG_RESULT(yes)
	  PYTHON_LIB="`$PYTHON -c 'import os,distutils.sysconfig;print distutils.sysconfig.get_python_lib(1).replace(os.sep,"/")'`"
	  AC_ARG_VAR(PYTHON_LIB, [Directory to install python bindings in])
	  PYTHON_PATHSEP="`python -c 'import os;print os.pathsep'`"
	  AC_SUBST(PYTHON_PATHSEP)
	  case "$host_os" in
	  mingw* | pw32*)
	    PYTHON_LIBS="`$PYTHON -c 'import os,sys;print "-L"+os.path.join(sys.prefix,"libs").replace(os.sep,"/"),"-lpython"+sys.version[[:3]].replace(".","")'`" ;;
	  *)
	    PYTHON_LIBS=
	  esac
	  AC_SUBST(PYTHON_LIBS)
	  BINDINGS="$BINDINGS python"
	else
	  AC_MSG_RESULT([no (install python-dev or python-devel package or similar)])
	fi
      fi
    fi
  fi

  if test no != "$with_php" ; then
    AC_PATH_PROG(PHP_CONFIG, [php-config], [])
    if test -n "$PHP_CONFIG" ; then
	case "`$PHP_CONFIG --version 2> /dev/null`" in
	4.*)
	    dnl If the php interpreter can't be found, skip the php tests.
	    AC_PATH_PROGS(PHP4, [php4 php], [\$(top_srcdir)/skiptest])
	    PHP_INC="`$PHP_CONFIG --includes`"
	    AC_SUBST(PHP_INC)
	    PHP_EXTENSION_DIR="`$PHP_CONFIG --extension-dir`"
	    AC_SUBST(PHP_EXTENSION_DIR)
	    BINDINGS="$BINDINGS php4"
	    ;;
	*) ;;
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
	    TCL_LIB="`echo 'puts $tcl_library'|$TCLSH`"
	    AC_SUBST(TCL_LIB)
	    dnl There's no obvious way to get "/usr/include/tcl8.N" directly...
	    dnl debian uses /usr/lib/tcl8.4, FC2 uses /usr/share/tcl8.4
	    TCL_INC="`echo \"$TCL_LIB\"|sed 's!/\(share\|lib\)/!/include/!'`"
	    dnl Check that the headers are there (the tcl8.N-dev package
	    dnl may not be installed).
	    TCL_CPPFLAGS=
	    if test -f "$TCL_INC/tcl.h" ; then
		BINDINGS="$BINDINGS tcl8"
		TCL_CPPFLAGS="-I$TCL_INC"
	    elif test -f "/usr/include/tcl.h" ; then
		if test x`awk '($1 == "#define" && $2 == "TCL_VERSION") {print $3}' /usr/include/tcl.h` = x'"'`echo 'puts $tcl_version'|$TCLSH`'"' ; then
		    BINDINGS="$BINDINGS tcl8"
		fi
	    fi
	    AC_SUBST(TCL_CPPFLAGS)
	fi
    fi
  fi

  if test yes == "$with_guile" ; then
    AC_PATH_PROG(GUILE, guile, [])
    if test -n "$GUILE" ;  then
	dnl test guile version?
	dnl any paths?
	BINDINGS="$BINDINGS guile"
    fi
  fi

  if test no != "$with_csharp" ; then
    AC_PATH_PROG(CSC, mcs, [])
    AC_PATH_PROG(GACUTIL, gacutil, [])
    AC_PATH_PROG(SN, sn, [])
    if test -n "$CSC" -a -n "$GACUTIL" -a -n "$SN" ; then
      BINDINGS="$BINDINGS csharp"
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
      AC_CHECK_HEADER(jni.h, [BINDINGS="$BINDINGS java"])
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
fi
AC_SUBST(SWIG_CXXFLAGS)
AC_SUBST(JAVA_CXXFLAGS)

AC_CONFIG_FILES([Makefile
 python/Makefile python/docs/Makefile python/docs/examples/Makefile
 php4/Makefile php4/docs/Makefile php4/docs/examples/Makefile php4/util.i
 java/Makefile java/native/Makefile java/org/Makefile java/org/xapian/Makefile
 java/org/xapian/errors/Makefile java/org/xapian/examples/Makefile
 guile/Makefile
 tcl8/Makefile tcl8/docs/Makefile tcl8/docs/examples/Makefile
 csharp/Makefile
 csharp/AssemblyInfo.cs
 xapian-bindings.spec])
AC_OUTPUT
