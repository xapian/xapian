## Process this file with autoconf to create configure. -*- autoconf -*-
# Copyright 2001-2005, 2007  Free Software Foundation, Inc.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
# 02110-1301  USA


dnl FIXME: Is this really new enough?
AC_PREREQ(2.50)



## ------------------------ ##
## Autoconf initialisation. ##
## ------------------------ ##
AC_INIT([libtool], [1.5.26], [bug-libtool@gnu.org])
AC_CONFIG_SRCDIR([ltmain.in])



## ---------------------------------------- ##
## Display a configure time version banner. ##
## ---------------------------------------- ##

# This is a sanity check so we can see which version is used in bug reports.
# It is assumed that we only want to see the date extension for cvs libtool
# versions (i.e. "odd" letters) and not actual alpha releases.
TIMESTAMP=
case AC_PACKAGE_VERSION in
  [*[acegikmoqsuwy])]
    TIMESTAMP=`${CONFIG_SHELL} ${srcdir}/mkstamp < ${srcdir}/ChangeLog`
    AS_BOX([Configuring AC_PACKAGE_TARNAME AC_PACKAGE_VERSION$TIMESTAMP])
    echo
    ;;
esac
AC_SUBST([TIMESTAMP])



## ------------------------ ##
## Automake Initialisation. ##
## ------------------------ ##
AM_INIT_AUTOMAKE(AC_PACKAGE_TARNAME, AC_PACKAGE_VERSION)


## ------------------------------- ##
## Libtool specific configuration. ##
## ------------------------------- ##
pkgdatadir='${datadir}'"/${PACKAGE}"
AC_SUBST([pkgdatadir]) # automake does not need this, but libtoolize does

aclocaldir='${datadir}/aclocal'
AC_SUBST([aclocaldir])

AC_ARG_ENABLE(ltdl-install,
    [AC_HELP_STRING([--disable-ltdl-install], [do not install libltdl])])
if test x"${enable_ltdl_install+set}" != xset; then
  enable_ltdl_install=yes
  ac_configure_args="$ac_configure_args --enable-ltdl-install"
fi
AC_CONFIG_SUBDIRS([libltdl])

# all subdirectories that are configured on demand, but that must be
# included in the distribution
CONF_SUBDIRS="cdemo pdemo demo depdemo mdemo mdemo2 tagdemo f77demo"
AC_SUBST([CONF_SUBDIRS])

ACINCLUDE_M4_LIST="${srcdir}/acinclude.m4"
DIST_MAKEFILE_LIST=
for dir in $CONF_SUBDIRS; do
  ACINCLUDE_M4_LIST="$ACINCLUDE_M4_LIST ${srcdir}/$dir/acinclude.m4"
  DIST_MAKEFILE_LIST="$DIST_MAKEFILE_LIST$dir/Makefile "
done
AC_SUBST([ACINCLUDE_M4_LIST])
AC_SUBST([DIST_MAKEFILE_LIST])



## ---------------- ##
## compiler checks. ##
## ---------------- ##

# Use the specified CC and LD
AC_PROG_CC
AC_EXEEXT
AM_PROG_LD
AC_SUBST(LD)
AM_PROG_NM
AC_SUBST(NM)
AC_PROG_LN_S

pushdef([AC_MSG_ERROR], [CXX=no])
AC_PROG_CXX
popdef([AC_MSG_ERROR])
AM_CONDITIONAL(HAVE_CXX, 
[test -n "$CXX" && ( test "X$CXX" != "Xno" &&
  ( (test "X$CXX" = "Xg++" && `g++ -v >/dev/null 2>&1` ) || 
  (test "X$CXX" != "Xg++")))])

pushdef([AC_MSG_ERROR], [F77=no])
AC_PROG_F77
popdef([AC_MSG_ERROR])
if test -z "$F77"; then
  F77="no"
fi
AM_CONDITIONAL(HAVE_F77, [test "x$F77" != xno])

LT_AC_PROG_GCJ
AM_CONDITIONAL(HAVE_GCJ, [test "x$GCJ" != xno])

LT_AC_PROG_RC
AM_CONDITIONAL(HAVE_RC, [test "x$RC" != xno])



## ----------------------- ##
## Libtool initialisation. ##
## ----------------------- ##
AC_LIBTOOL_DLOPEN
AC_LIBTOOL_WIN32_DLL
AC_PROG_LIBTOOL



## -------- ##
## Outputs. ##
## -------- ##
AC_CONFIG_FILES([Makefile doc/Makefile tests/Makefile])
AC_OUTPUT
