# Process this file with autoconf to produce a configure script.

# Copyright (C) 1995, 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2006,
# 2004, 2006, 2007, 2008  Free Software Foundation, Inc.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
# 02110-1301, USA.

dnl Require Autoconf 2.60 for AC_REQUIRE_AUX_FILE.
AC_PREREQ(2.60)
AC_INIT([GNU Automake], [1.10.1], [bug-automake@gnu.org])

AC_CONFIG_SRCDIR(automake.in)
AC_CONFIG_AUX_DIR(lib)

AC_CANONICAL_BUILD

# Save the AUTOCONF setting before AM_INIT_AUTOMAKE overrides it; this
# way we can run Autoconf tests from configure (or from the test
# suite) without being bothered by `missing'.
AC_SUBST([am_AUTOCONF], ["${AUTOCONF-autoconf}"])

AM_INIT_AUTOMAKE([1.8a dist-bzip2 filename-length-max=99])

# The API version is the base version.  We must guarantee
# compatibility for all releases with the same API version.
# Our current rule is that:
# * All releases, including the prereleases, in an X.Y series
#   are compatible.  So 1.5.1c is compatible with 1.5.
# * Prereleases on the trunk are all incompatible -- 1.5b and 1.5c
#   aren't the same.
APIVERSION=`echo "$VERSION" | sed -e 's/^\([[0-9]]*\.[[0-9]]*[[a-z]]*\).*$/\1/'`
AC_SUBST(APIVERSION)

# A versioned directory, defined here for convenience.
AC_SUBST([pkgvdatadir], ["\${datadir}/$PACKAGE-$APIVERSION"])

# $AUTOMAKE and $ACLOCAL are always run after a `cd $top_srcdir',
# hence `.' is really what we want for perllibdir, libdir, and acdir.
ACLOCAL="perllibdir=`pwd`\"/lib$PATH_SEPARATOR./lib\" `pwd`/aclocal --acdir=m4 -I m4"
AUTOMAKE="perllibdir=`pwd`\"/lib$PATH_SEPARATOR./lib\" `pwd`/automake --libdir=lib"

AC_PATH_PROG(PERL, perl)
if test -z "$PERL"; then
   AC_MSG_ERROR([perl not found])
fi
$PERL -e 'require 5.006;' || {
   AC_MSG_ERROR(
[perl 5.6 or better is required; perl 5.8.2 or better
is recommended.  If you have several perl versions
installed, select the one Automake should use using
  ./configure PERL=/path/to/perl])
}

# The test suite will skip some tests if tex is absent.
AC_CHECK_PROG([TEX], [tex], [tex])

# Test for Autoconf.  We run Autoconf in a subdirectory to ease
# deletion of any files created (such as those added to
# autom4te.cache).  We used to perform only the last of the three
# following tests, but some users were unable to figure out that their
# installation was broken since --version appeared to work.

AC_CACHE_CHECK([whether autoconf is installed], [am_cv_autoconf_installed],
[if AM_RUN_LOG([eval $am_AUTOCONF --version]);
then
  am_cv_autoconf_installed=yes
else
  am_cv_autoconf_installed=no
fi])
if test "$am_cv_autoconf_installed" = no; then
  AC_MSG_ERROR([Autoconf 2.60 or better is required.
    Please make sure it is installed and in your PATH.])
fi

AC_CACHE_CHECK([whether autoconf works], [am_cv_autoconf_works],
[mkdir conftest
echo 'AC''_INIT' > conftest/conftest.ac
if AM_RUN_LOG([cd conftest && eval $am_AUTOCONF -o /dev/null conftest.ac]);
then
  am_cv_autoconf_works=yes
else
  am_cv_autoconf_works=no
fi
rm -rf conftest])
if test "$am_cv_autoconf_works" = no; then
  AC_MSG_ERROR([The installed version of autoconf does not work.
    Please check config.log for error messages before this one.])
fi

AC_CACHE_CHECK([whether autoconf is recent enough], [am_cv_autoconf_version],
[mkdir conftest
echo 'AC''_PREREQ(2.60)' > conftest/conftest.ac
if AM_RUN_LOG([cd conftest && eval $am_AUTOCONF -o /dev/null conftest.ac]);
then
  am_cv_autoconf_version=yes
else
  am_cv_autoconf_version=no
fi
rm -rf conftest])
if test "$am_cv_autoconf_version" = no; then
  AC_MSG_ERROR([Autoconf 2.60 or better is required.])
fi

# Test for ln.  We need use it to install the versioned binaries.
AC_MSG_CHECKING([whether ln works])
AC_CACHE_VAL([am_cv_prog_ln], [
rm -f conftest conftest.file
: >conftest.file
if ln conftest.file conftest 2>/dev/null; then
  am_cv_prog_ln=ln
else
  am_cv_prog_ln='cp -p'
fi
rm -f conftest conftest.file])
AC_SUBST([LN], [$am_cv_prog_ln])
result=no
test "x$am_cv_prog_ln" = xln && result=yes
AC_MSG_RESULT([$result])

# The amount we should wait after modifying files depends on the platform.
# On Windows '95, '98 and ME, files modifications have 2-seconds
# granularity and can be up to 3 seconds in the future w.r.t. the
# system clock.  When it is important to ensure one file is older
# than another we wait at least 5 seconds between creations.
case $build in
  *-pc-msdosdjgpp) MODIFICATION_DELAY=5;;
  *)               MODIFICATION_DELAY=2;;
esac
AC_SUBST([MODIFICATION_DELAY])

# Test for things needed by the test suite.
AC_PROG_EGREP
AC_PROG_FGREP

AC_CONFIG_FILES([
  Makefile
  doc/Makefile
  lib/Automake/Makefile
  lib/Automake/tests/Makefile
  lib/Makefile
  lib/am/Makefile
  m4/Makefile
  tests/Makefile
  tests/defs
])
AC_CONFIG_FILES([tests/aclocal-${APIVERSION}:tests/aclocal.in],
                [chmod +x tests/aclocal-${APIVERSION}],
	        [APIVERSION=$APIVERSION])
AC_CONFIG_FILES([tests/automake-${APIVERSION}:tests/automake.in],
                [chmod +x tests/automake-${APIVERSION}])

AC_OUTPUT
