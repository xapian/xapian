# Process this file with autoconf to create configure. -*- autoconf -*-

#   Copyright (C) 2001, 2004, 2005, 2008 Free Software Foundation, Inc.
#   Written by Gary V. Vaughan, 2001
#
#   This file is part of GNU Libtool.
#
# GNU Libtool is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# GNU Libtool is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with GNU Libtool; see the file COPYING.  If not, a copy
# can be downloaded from  http://www.gnu.org/licenses/gpl.html,
# or obtained by writing to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
####


AC_PREREQ(2.59)dnl We use AS_HELP_STRING
dnl Oldest automake required for bootstrap is below in AM_INIT_AUTOMAKE.


## ------------------------ ##
## Autoconf initialisation. ##
## ------------------------ ##
AC_INIT([libtool], [2.2.4], [bug-libtool@gnu.org])
AC_CONFIG_HEADERS([config.h:config-h.in])
AC_CONFIG_SRCDIR([libtoolize.in])
LT_CONFIG_LTDL_DIR([libltdl], [nonrecursive])
AC_CONFIG_AUX_DIR([libltdl/config])
AC_CONFIG_MACRO_DIR([libltdl/m4])
AC_CONFIG_LIBOBJ_DIR([libltdl])


## ------------------------ ##
## Autotest initialisation. ##
## ------------------------ ##

AC_CONFIG_TESTDIR([tests])
AC_SUBST([AUTOM4TE], [${AUTOM4TE=autom4te}])
AC_SUBST([AUTOTEST], ['$(AUTOM4TE) --language=autotest'])


## ---------------------------------------- ##
## Display a configure time version banner. ##
## ---------------------------------------- ##

# Calculating the previous version is somewhat involved, so do it once
# here, and SUBST the result for the maintainer release rules.  The
# versioning scheme we use is described here:
#	http://www.gnu.org/software/libtool/contribute.html

# First we break the version number up into its constituent parts.
lt_major=`echo "AC_PACKAGE_VERSION" | sed ['s,\..*$,,g']`
lt_minor=`echo "AC_PACKAGE_VERSION" | sed ['s,^[0-9]*\.\([0-9]*\).*$,\1,']`
lt_micro=`echo "AC_PACKAGE_VERSION" | sed ['s,^[0-9]*\.[0-9]*\(\.[0-9]*\).*$,\1,']`
lt_alpha=`echo "AC_PACKAGE_VERSION" | sed ['s,^[0-9.]*,,']`

test "$lt_minor" = "AC_PACKAGE_VERSION" && lt_minor=0
test "$lt_micro" = "AC_PACKAGE_VERSION" && lt_micro=""

# Then we try to work out what the release before this one would have been
# numbered.  The only time we come unstuck is when this is the first release
# from a stable branch (lt_micro=lt_alpha=""), so LASTRELEASE is the last
# release from the old stable branch, and we don't know what version that
# was, so leave it unset so that the Makefile can complain.
case $lt_alpha in
  [[ab]])
    case $lt_micro in
      [.[01]])
	LASTRELEASE="$lt_major.$lt_minor" ;;
      *)
	LASTRELEASE="$lt_major.$lt_minor$lt_micro" ;;
    esac
    ;;
  [[cegikmoqsuwy]])
    LASTRELEASE=`echo "AC_PACKAGE_VERSION" | tr 'c-y' 'b-w'` ;;
  [[dfhjlnprtvxz]])
    LASTRELEASE=`echo "AC_PACKAGE_VERSION" | tr 'd-z' 'b-x'` ;;
  *) # No lt_alpha component
    case $lt_micro in
      .0)
	LASTRELEASE="" ;;
      .1)
        LASTRELEASE="$lt_major.$lt_minor" ;;
      .*)
	lt_micro_value=`echo $lt_micro | sed 's,^\.,,'`
	LASTRELEASE="$lt_major.$lt_minor.`expr $lt_micro_value - 1`" ;;
      *)
	LASTRELEASE="" ;;
    esac
    ;;
esac
AC_SUBST([LASTRELEASE])

# This is a sanity check so we can see which version is used in bug reports.
# It is assumed that we only want to see the date extension for cvs libtool
# versions (i.e. "odd" letters) and not actual alpha releases.
TIMESTAMP=`${CONFIG_SHELL} ${ac_aux_dir}/mkstamp ${srcdir}`
package_revision=`( set $TIMESTAMP; echo $1; )`
case $lt_alpha in
  [[bdfhjlnprtvxz]])
    TIMESTAMP=
    ;;
esac
AS_BOX([Configuring AC_PACKAGE_TARNAME${TIMESTAMP:+" (Build:$TIMESTAMP)"} AC_PACKAGE_VERSION])
echo
AC_SUBST([TIMESTAMP])
AC_SUBST([package_revision])


## ------------------------ ##
## Automake Initialisation. ##
## ------------------------ ##

dnl These are bootstrap requirements, once built, libtool may work with
dnl much older releases of autoconf and automake.  See release notes.
AM_INIT_AUTOMAKE([1.9 gnu subdir-objects dist-bzip2])

dnl We use m4sh to generate libtool's portable shell scripts
AC_SUBST([M4SH], ['$(AUTOM4TE) --language=m4sh'])

dnl Make sure config.status is regenerated when the version timestamp changes
AC_SUBST([CONFIG_STATUS_DEPENDENCIES], ['$(top_srcdir)/ChangeLog'])

dnl automake-1.9.x or autoconf-2.59 don't support subdir libobjs
AC_MSG_CHECKING([whether subdir libobjs are useable])
test -f $srcdir/argz.c || sublibobjs_supported=yes
AM_CONDITIONAL([LTDL_SUBDIR_LIBOBJS], test x"${sublibobjs_supported-no}" != xno)
AC_MSG_RESULT([${sublibobjs_supported-no}])


## ------------------------------- ##
## Libtool specific configuration. ##
## ------------------------------- ##

dnl automake-1.9 does not substitute these two by default
AC_SUBST([abs_top_srcdir])
AC_SUBST([abs_top_builddir])
AC_SUBST([aclocaldir], ["\${datadir}/aclocal"])
AC_SUBST([pkgdatadir], ["\${datadir}/$PACKAGE"])

AC_ARG_ENABLE(ltdl-install,
    [AS_HELP_STRING([--disable-ltdl-install], [do not install libltdl])])
if test x"${enable_ltdl_install+set}" != xset; then
  enable_ltdl_install=yes
fi

# All subdirectories that are configured on demand, but that must be
# included in the distribution.
CONF_SUBDIRS="tests/cdemo tests/demo tests/depdemo tests/f77demo tests/fcdemo \
	tests/mdemo tests/mdemo2 tests/pdemo tests/tagdemo"
AC_SUBST([CONF_SUBDIRS])

DIST_MAKEFILE_LIST=
for dir in $CONF_SUBDIRS; do
  DIST_MAKEFILE_LIST="${DIST_MAKEFILE_LIST}$dir/Makefile "
done
AC_SUBST([DIST_MAKEFILE_LIST])


## ---------------- ##
## compiler checks. ##
## ---------------- ##

# Need to be sure that we have a working C compiler, otherwise even with
# a good C compiler: if the following C++ tests fail, the whole libtool
# build fails!
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_CPP

## ----------------------- ##
## Libtool initialisation. ##
## ----------------------- ##
LT_INIT([dlopen win32-dll])
_LT_SET_OPTION([LTDL_INIT], [nonrecursive])
_LTDL_SETUP

# Enable all the language support we can
LT_LANG(C++)
LT_LANG(Fortran 77)
LT_LANG(Fortran)
LT_LANG(Java)
LT_LANG(Windows Resource)


## --------------------------- ##
## Work out which tests to run ##
## --------------------------- ##
AM_CONDITIONAL(HAVE_CXX,[test -n "[$]_LT_TAGVAR(compiler, CXX)"])
AM_CONDITIONAL(HAVE_F77,[test -n "[$]_LT_TAGVAR(compiler, F77)"])
AM_CONDITIONAL(HAVE_FC,[test -n "[$]_LT_TAGVAR(compiler, FC)"])
AM_CONDITIONAL(HAVE_GCJ, [test -n "[$]_LT_TAGVAR(compiler, GCJ)"])
AM_CONDITIONAL(HAVE_RC, [test -n "[$]_LT_TAGVAR(compiler, RC)"])


## -------- ##
## Outputs. ##
## -------- ##
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
