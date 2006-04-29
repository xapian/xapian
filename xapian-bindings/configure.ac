dnl Process this file with autoconf to produce a configure script.

dnl Need autoconf 2.50 or later for many features
dnl 2.54 needed for automake 1.6
dnl 2.57 fixes the annoying warnings from configure on FreeBSD
dnl 2.58 is needed by automake 1.8.5
dnl 2.59 was released the same day as 2.58 to fix a problem
AC_PREREQ(2.59)
AC_INIT(xapian-bindings, 0.9.5)dnl FIXME: bugreport addr as third argument
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
OVERRIDE_MACOSX_DEPLOYMENT_TARGET=
case $host_os in
darwin*)
  case $MACOSX_DEPLOYMENT_TARGET in
  "")
    OVERRIDE_MACOSX_DEPLOYMENT_TARGET=10.3 ;;
  10.[[012]])
    AC_MSG_WARN([Overriding MACOSX_DEPLOYMENT_TARGET from $MACOSX_DEPLOYMENT_TARGET to 10.3])
    OVERRIDE_MACOSX_DEPLOYMENT_TARGET=10.3 ;;
  esac ;;
esac
AM_CONDITIONAL(OVERRIDE_MACOSX_DEPLOYMENT_TARGET, test -n "$OVERRIDE_MACOSX_DEPLOYMENT_TARGET")
AC_SUBST(OVERRIDE_MACOSX_DEPLOYMENT_TARGET)
if test -n "$OVERRIDE_MACOSX_DEPLOYMENT_TARGET" ; then
  MACOSX_DEPLOYMENT_TARGET=$OVERRIDE_MACOSX_DEPLOYMENT_TARGET
fi

dnl Use libtool to manage our libraries, but don't build static libraries as
dnl the bindings only have a use for dynamic ones.
AC_DISABLE_STATIC
AC_PROG_LIBTOOL

dnl -no-undefined causes problems on MacOS X with some at least some
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
  dnl AC_PATH_PROGS only honours an already set SWIG is it's a full
  dnl path.  Listing it in the "to be searched" list like this allows
  dnl ./configure SWIG=myswig to work.
  AC_PATH_PROGS(SWIG, [$SWIG swig], [])
  AC_ARG_VAR(SWIG, [SWIG interface generator])
  if test -z "$SWIG" ; then
    AC_MSG_ERROR([Can't find SWIG utility])
  fi
  dnl Check for swig >= 1.3.29
  dnl 1.3.23 adds support for optional arguments in C#.
  dnl 1.3.24 tweaks C# enums
  dnl 1.3.26 fixes PHP overloading
  dnl 1.3.27 contains a few fixes, none of which affect us
  dnl 1.3.28 contains many useful fixes and changes - one thing we need it
  dnl for is a ability to automatically rename method names for C# (e.g.
  dnl get_description becomes getDescription).
  dnl 1.3.29 fixes PHP to give an error if an overload can't be resolved
  dnl You can see the latest SWIG changes here:
  dnl http://cvs.sourceforge.net/viewcvs.py/*checkout*/swig/SWIG/CHANGES
  v=`$SWIG -version 2>&1|sed 's/^SWIG Version \([[0-9\.]]*\).*/\1/p;d'`
  case $v in
   [1.3.29|1.3.[3-9][0-9]|1.3.[1-9][0-9][0-9][0-9]*|1.[4-9].*|2.*]) ;;
   *)
     AC_MSG_ERROR([SWIG >= 1.3.29 required (you have $v)]) ;;
  esac
  SWIG_FLAGS="`$XAPIAN_CONFIG --swigflags` -Werror"
  AC_SUBST(SWIG_FLAGS)
fi

AC_ARG_WITH(swig,
  AC_HELP_STRING([--with-swig],
		 [enable all bindings which depend on SWIG (currently: Python PHP Tcl Guile CSharp Ruby)]),
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

AC_ARG_WITH(ruby,
  AC_HELP_STRING([--with-ruby], [enable Ruby bindings]),
  [],
  [with_ruby=$with_swig])

AC_ARG_WITH(tcl,
  AC_HELP_STRING([--with-tcl], [enable Tcl bindings]),
  [],
  [with_tcl=$with_swig])

dnl Guile support doesn't currently work, so avoid user disappointment by
dnl not advertising it in "./configure --help"
dnl AC_ARG_WITH(guile,
dnl   AC_HELP_STRING([--with-guile], [enable Guile bindings]),
dnl   [],
dnl   [with_guile=$with_swig])
with_guile=no

AC_ARG_WITH(csharp,
  AC_HELP_STRING([--with-csharp], [enable CSharp bindings]),
  [],
  [with_csharp=$with_swig])

AC_ARG_WITH(java,
  AC_HELP_STRING([--with-java], [enable Java bindings]),
  [],
  [with_java=])

case $with_swig$with_python$with_php$with_ruby$with_tcl$with_guile$with_csharp$with_java in
*yes*)
  dnl Default unspecified values to no.
  test -z "$with_python" && with_python=no
  test -z "$with_php" && with_php=no
  test -z "$with_tcl" && with_tcl=no
  test -z "$with_guile" && with_guile=no
  test -z "$with_csharp" && with_csharp=no
  test -z "$with_java" && with_java=no
  test -z "$with_ruby" && with_ruby=no
  ;;
esac

BINDINGS=

if test no != "$with_python" ; then
  dnl See comment for AC_PATH_PROGS(SWIG, ...).
  AC_PATH_PROGS(PYTHON, [$PYTHON python], [])
  AC_ARG_VAR(PYTHON, [Python interpreter])
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
      dnl Use output from swig -modern for Python 2.2 and up for
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
      if $PYTHON -c 'import distutils.sysconfig' 2>/dev/null ; then
	PYTHON_INC=`$PYTHON -c 'import os,distutils.sysconfig;print distutils.sysconfig.get_python_inc().replace(os.sep,"/")'`
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
	PYTHON_LIB=`$PYTHON -c 'import os,distutils.sysconfig;print distutils.sysconfig.get_python_lib(1).replace(os.sep,"/")'`
	AC_ARG_VAR(PYTHON_LIB, [Directory to install python bindings in])
	PYTHON_PATHSEP=`python -c 'import os;print os.pathsep'`
	AC_SUBST(PYTHON_PATHSEP)
	case $host_os in
	mingw* | pw32*)
	  PYTHON_LIBS=`$PYTHON -c 'import os,sys;print "-L"+os.path.join(sys.prefix,"libs").replace(os.sep,"/"),"-lpython"+sys.version[[:3]].replace(".","")'` ;;
	*)
	  PYTHON_LIBS= ;;
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
  dnl See comment for AC_PATH_PROGS(SWIG, ...).
  AC_PATH_PROGS(PHP_CONFIG, [$PHP_CONFIG php-config], [])
  AC_ARG_VAR(PHP_CONFIG, [php-config utility])
  AC_ARG_VAR(PHP, [php interpreter to optionally run testsuite with])
  if test -n "$PHP_CONFIG" ; then
    version=`$PHP_CONFIG --version 2> /dev/null`
    case $version in
    4.*)
      dnl If the php interpreter can't be found, skip the php tests.
      AC_PATH_PROGS(PHP, [$PHP php4 php], [\$(top_srcdir)/skiptest])
      php_version=4
      ;;
    5.*)
      dnl If the php interpreter can't be found, skip the php tests.
      AC_PATH_PROGS(PHP, [$PHP php5 php], [\$(top_srcdir)/skiptest])
      php_version=5
      ;;
    *)
      if test yes = "$with_php" ; then
	AC_MSG_ERROR([Only PHP4 and PHP5 are supported ($PHP_CONFIG reports version $version)])
      fi
      with_php=no
      ;;
    esac
    if test no != "$with_php" ; then
      PHP_INC=`$PHP_CONFIG --includes`
      AC_SUBST(PHP_INC)
      PHP_EXTENSION_DIR=`$PHP_CONFIG --extension-dir`
      AC_SUBST(PHP_EXTENSION_DIR)
      case $host_os in
      mingw* | pw32*)
	dnl This is a bit of an informed guess, pending more information from
	dnl someone who actually has mingw and wants to build the PHP bindings
	dnl there.  FIXME.
	PHP_LIBS="-L`$PHP_CONFIG --prefix` -lphp${php_version}ts" ;;
      *)
	PHP_LIBS= ;;
      esac
      AC_SUBST(PHP_LIBS)
      BINDINGS="$BINDINGS php"
    fi
  fi
fi

if test no != "$with_tcl" ; then
  AC_PATH_PROGS(TCLSH, [$TCLSH tclsh], [])
  AC_ARG_VAR(TCLSH, [Tcl interpreter])
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
  AC_PATH_PROGS(GUILE, [$GUILE guile], [])
  AC_ARG_VAR(GUILE, [guile interpreter])
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
  AC_PATH_PROGS(CSC, [$CSC mcs], [])
  AC_ARG_VAR(CSC, [C# compiler command])
  if test -n "$CSC" ; then
    # There are other tools called mcs (such as /usr/bin/mcs on Tru64),
    # so we check that the mcs we found understands --version which is
    # sufficient to distinguish mono's mcs from the Tru64 one.
    AC_MSG_CHECKING([whether $CSC is from GNU Mono])
    if (exec >&5 2>&5;$CSC --version </dev/null;exit $?) ; then
      AC_MSG_RESULT(yes)
      AC_PATH_PROGS(GACUTIL, [$GACUTIL gacutil], [])
      AC_PATH_PROGS(SN, [$SN sn], [])
      AC_ARG_VAR(GACUTIL, [gacutil utility to use for C# bindings])
      AC_ARG_VAR(SN, [sn utility to use for C# bindings])
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
  AC_PATH_PROGS(JAVA, [$JAVA java], [],
  	[${JAVA_HOME+$JAVA_HOME/bin:}${JDK_HOME+$JDK_HOME/bin:}$PATH])
  AC_PATH_PROGS(JAVAC, [$JAVAC javac], [],
  	[${JAVA_HOME+$JAVA_HOME/bin:}${JDK_HOME+$JDK_HOME/bin:}$PATH])
  AC_PATH_PROGS(JAR, [$JAR jar], [],
  	[${JAVA_HOME+$JAVA_HOME/bin:}${JDK_HOME+$JDK_HOME/bin:}$PATH])
  AC_ARG_VAR(JAVA, [Java interpreter command])
  AC_ARG_VAR(JAVAC, [Java compiler command])
  AC_ARG_VAR(JAR, [java jar utility])
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

if test no != "$with_ruby" ; then
  dnl See comment for AC_PATH_PROGS(SWIG, ...).
  AC_PATH_PROGS(RUBY, [$RUBY ruby], [])
  AC_ARG_VAR(RUBY, [Ruby interpreter])
  if test -n "$RUBY" ; then
    dnl Require Ruby 1.8 or newer.  Paul says:
    dnl  I'm using 1.8.4.  I imagine anything in the 1.8.x series will be fine,
    dnl  not sure about 1.6 but hardly anyone uses <1.8.
    dnl
    dnl Brief testing with Ruby 1.6.8 show the bindings probably work there,
    dnl but smoketest.rb doesn't because the test/unit module isn't available.
    version=`$RUBY --version 2>/dev/null`
    case $version in
    [ruby\ 1.[89]*]) ;; # Ruby 1.8, 1.9
    [ruby\ 1.[1-9][0-9]*]) ;; # Ruby 1.10+
    [ruby\ [2-9]*]) ;; # Ruby 2-Ruby 9
    [ruby\ 1[0-9]*]) ;; # Ruby 10+
    *)
      if test yes = "$with_ruby" ; then
	AC_MSG_ERROR([Only Ruby 1.8 or newer is supported ($ruby is $version)])
      fi
      RUBY=
      ;;
    esac
    if test -n "$RUBY" ; then
      RUBY_INC=`$RUBY -rrbconfig -e 'puts Config::CONFIG[["archdir"]]'`
      AC_SUBST(RUBY_INC)
      dnl Check that ruby.h is there, which is a good way to check that
      dnl the appropriate ruby-dev package has been installed.
      AC_MSG_CHECKING([for $RUBY_INC/ruby.h])
      if test -f "$RUBY_INC/ruby.h" ; then
	AC_MSG_RESULT(yes)
	RUBY_LIB=`$RUBY -rrbconfig -e 'puts Config::CONFIG[["rubylibdir"]]'`
	AC_SUBST(RUBY_LIB)
	dnl Ruby seems to keep ruby.h in the same directory where C/C++
	dnl extensions shared libraries go.
	RUBY_LIB_ARCH=$RUBY_INC
	AC_SUBST(RUBY_LIB_ARCH)
	BINDINGS="$BINDINGS ruby"
      else
	AC_MSG_RESULT([no (install ruby-dev or ruby-devel package or similar)])
	if test yes = "$with_ruby" ; then
	  AC_MSG_ERROR([ruby.h not found])
	fi
      fi
    fi
  fi
fi

AC_SUBST(BINDINGS)

dnl Set flags to control warnings (enable more, or disable annoying ones)
dnl and other compiler specific flags.
AM_CXXFLAGS=
SWIG_CXXFLAGS=
JAVA_CXXFLAGS=
if test yes = "$GXX" ; then
  dnl Python itself is compiled with -fno-strict-aliasing, and it appears
  dnl it's safest to follow this lead when compiling the SWIG generated
  dnl interface code.  E.g.:
  dnl   http://article.gmane.org/gmane.comp.gcc.devel/74692
  PYTHON_CXXFLAGS="-fno-strict-aliasing"

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
      SWIG_CXXFLAGS="-Wall -w1 -wd177,1572"
      dnl The Java JNI wrappers have a few unused variables.
      JAVA_CXXFLAGS="-Wall -w1 -wd177,1572"
    ],
    [
      dnl GCC:
      dnl
      dnl Swig generated code gives lots of unused and uninitialized warnings.
      dnl They're non-harmful, so suppress them.
      SWIG_CXXFLAGS="-Wall -Wno-unused -Wno-uninitialized"
      dnl The Java JNI wrappers have a few unused variables.
      JAVA_CXXFLAGS="-Wall -W -Wno-unused"
    ])
fi
AC_SUBST(SWIG_CXXFLAGS)
AC_SUBST(JAVA_CXXFLAGS)

AC_CONFIG_FILES([Makefile
 xapian-version.h
 python/Makefile python/docs/Makefile
 php/Makefile php/docs/Makefile
 java/Makefile java/native/Makefile java/org/Makefile java/org/xapian/Makefile
 java/org/xapian/errors/Makefile java/org/xapian/examples/Makefile
 guile/Makefile
 tcl8/Makefile tcl8/docs/Makefile
 csharp/Makefile csharp/docs/Makefile
 csharp/AssemblyInfo.cs
 ruby/Makefile ruby/docs/Makefile
 xapian-bindings.spec])
AC_OUTPUT

echo ""
echo "*** Building bindings for languages:" $BINDINGS
echo ""
