dnl Process this file with autoconf to produce a configure script.

dnl Need autoconf 2.50 or later for many features
dnl 2.54 needed for automake 1.6
dnl 2.57 fixes the annoying warnings from configure on FreeBSD
dnl 2.58 is needed by automake 1.8.5
dnl 2.59 was released the same day as 2.58 to fix a problem
AC_PREREQ(2.59)
AC_INIT(xapian-bindings, 0.8.3)dnl FIXME: bugreport addr as third argument
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR(xapian.i)

AM_CONFIG_HEADER(config.h)

dnl Use libtool to manage our libraries, but don't build static libraries as
dnl the bindings only have a use for dynamic ones.
AC_DISABLE_STATIC
AC_PROG_LIBTOOL

dnl Checks for programs.
AC_PROG_CC
AC_PROG_CXX

dnl Check for libxapian
XO_LIB_XAPIAN(, AC_MSG_ERROR([Can't find Xapian library]))

AC_ARG_WITH(swig,
  [  --without-swig          disable all bindings which depend on SWIG
                         (currently: Python PHP Tcl Guile)])
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
  [  --with-csharp           enable CSharp bindings],
  [with_csharp=$withval],
  with_csharp=no)

BINDINGS=

if test no != "$with_swig" ; then
  dnl Check for swig - this does most of the work for the bindings.
  AC_PATH_PROG(SWIG, swig, [])
  if test -z "$SWIG" ; then
    AC_MSG_ERROR([Can't find SWIG utility])
  fi
  dnl Check for swig >= 1.3.20
  dnl James says:
  dnl  1.3.18 -> 1.3.19 was very quick, and 1.3.15 through 1.3.17
  dnl  were mostly bugfixes on 1.3.14, so 1.3.14 might be sufficient. 1.3.12
  dnl  introduced support for C++ namespaces; you need 1.3.14 for overloaded
  dnl  functions (which we use).
  dnl swig 1.3.19 was missing std_common.i - it was empty at the time so you
  dnl can just touch it into existence, but it's cleaner to just require
  dnl 1.3.20.
  dnl 1.3.21 fixes a few minor issues which don't look to be relevant to
  dnl us currently.  1.3.22 has lots of changes, but again nothing looks
  dnl relevant to us currently.  For details see:
  dnl http://cvs.sourceforge.net/viewcvs.py/*checkout*/swig/SWIG/CHANGES
  v="`$SWIG -version 2>&1|sed 's/^SWIG Version \([[0-9\.]]*\).*/\1/p;d'`"
  case "$v" in
   [1.3.[2-9][0-9]|1.3.[1-9][0-9][0-9]*|1.[4-9].*|2.*]) ;;
   *)
     AC_MSG_ERROR([SWIG >= 1.3.20 required (you have $v)]) ;;
  esac

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
	SWIG_PYTHON_FLAGS=
	;;
      [2.[1-9]*|[3-9].*])
	dnl Call swig -modern for Python 2.2 and up for cleaner, leaner, faster
	dnl wrapper code in xapian.py.
	SWIG_PYTHON_FLAGS=-modern
	;;
      *)
	PYTHON=
	;;
      esac
      if test -n "$PYTHON" ; then
	AC_SUBST(SWIG_PYTHON_FLAGS)
	PYTHON_INC="`$PYTHON -c 'import sys;print sys.prefix + "/include/python" + sys.version[[:3]]'`"
	AC_SUBST(PYTHON_INC)
	PYTHON_LIB="`$PYTHON -c 'import sys;print sys.prefix + "/lib/python" + sys.version[[:3]]'`"
	AC_SUBST(PYTHON_LIB)
	BINDINGS="$BINDINGS python"
      fi
    fi
  fi

  if test no != "$with_php" ; then
    AC_PATH_PROG(PHP_CONFIG, php-config, [])
    if test -n "$PHP_CONFIG" ; then
	case "`$PHP_CONFIG --version 2> /dev/null`" in
	4.*)
	    AC_PATH_PROG(PHP4, php4, [])
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
	if `echo 'if {$tcl_version < 8.0 } { exit 1 }'|$TCLSH 2> /dev/null` ; then
	    TCL_LIB="`echo 'puts $tcl_library'|$TCLSH`"
	    AC_SUBST(TCL_LIB)
	    dnl There's no obvious way to get "/usr/include/tcl8.2" directly...
	    dnl debian uses /usr/lib/tcl8.4, FC2 uses /usr/share/tcl8.4
	    TCL_INC="`echo \"$TCL_LIB\"|sed 's!/\(share\|lib\)/!/include/!'`"
	    dnl Check that the headers are there (the tcl8.2-dev package
	    dnl may not be installed).
	    TCL_CPPFLAGS=
	    if test -f "$TCL_INC/tcl.h" ; then
		BINDINGS="$BINDINGS tcl8"
		TCL_CPPFLAGS="-I$TCL_INC"
	    elif test -f "/usr/include/tcl.h" ; then
		if test `awk '($1 == "#define" && $2 == "TCL_VERSION") {print $3}' /usr/include/tcl.h` = \"`echo 'puts $tcl_version'|$TCLSH`\" ; then
		    BINDINGS="$BINDINGS tcl8"
		fi
	    fi
	    AC_SUBST(TCL_CPPFLAGS)
	fi
    fi
  fi

  if test no != "$with_guile" ; then
    AC_PATH_PROG(GUILE, guile, [])
    dnl FIXME: uncomment once guile is building again... 
    dnl if test -n "$GUILE" ;  then
	dnl test guile version?
	dnl any paths?
	dnl BINDINGS="$BINDINGS guile"
    dnl fi
  fi

  if test no != "$with_csharp" ; then
    AC_PATH_PROG(CSC, mcs, [])
    if test -n "$CSC" ;  then
      AC_PATH_PROG(GACUTIL, gacutil, no)
      if test "x$GACUTIL" = "xno" ; then
        AC_MSG_ERROR([No gacutil tool found])
      fi
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
if test yes = "$GCC" ; then
  dnl Swig generated code gives lots of unused and uninitialized warnings.
  dnl They're non-harmful, so suppress them.
  SWIG_CXXFLAGS="-Wall -Wno-unused -Wno-uninitialized"
  JAVA_CXXFLAGS="-Wall -W -Wunused"
fi
AC_SUBST(SWIG_CXXFLAGS)
AC_SUBST(JAVA_CXXFLAGS)

AC_CONFIG_FILES([Makefile
 python/Makefile python/docs/Makefile python/docs/examples/Makefile
 php4/Makefile php4/docs/Makefile php4/docs/examples/Makefile php4/util.i
 java/Makefile java/native/Makefile java/org/Makefile java/org/xapian/Makefile
 java/org/xapian/errors/Makefile java/org/xapian/examples/Makefile
 guile/Makefile
 tcl8/Makefile
 csharp/Makefile
 csharp/AssemblyInfo.cs
 xapian-bindings.spec])
AC_OUTPUT
