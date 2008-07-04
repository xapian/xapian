# Process this file with autoconf to create configure. -*- autoconf -*-
#
#   Copyright (C) 2001, 2003, 2005, 2007 Free Software Foundation, Inc.
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

AC_PREREQ(2.54)

## ------------------------ ##
## Autoconf initialisation. ##
## ------------------------ ##
AC_INIT([fcdemo], [1.0], [bug-libtool@gnu.org])
AC_CONFIG_HEADERS([config.h:config-h.in])
AC_CONFIG_SRCDIR([foof.f90])
AC_CONFIG_AUX_DIR([../../libltdl/config])

AC_ARG_WITH([dist], AS_HELP_STRING([--with-dist],
				   [ignore errors not affecting `make dist']))

## ------------------------ ##
## Automake Initialisation. ##
## ------------------------ ##
AM_INIT_AUTOMAKE


## ------------------ ##
## C compiler checks. ##
## ------------------ ##
AC_PROG_CC

## ------------------------ ##
## Fortran compiler checks. ##
## ------------------------ ##
AC_LANG_PUSH([Fortran])
AC_PROG_FC
dnl When configuring for `make dist' purposes, skip checks that may yield fatal
dnl errors when there is no working F90 compiler.
if test -z "$with_dist"; then
  dnl Check the flags needed to link fc programs with ld (i.e. cc)
  AC_FC_LIBRARY_LDFLAGS
  dnl Check for underscoring of external names
  AC_FC_WRAPPERS
  dnl We need to use .f90 and not .f to enable Automake FC support
  dnl Some Intel ifc/ifort do not understand .f95.  :-/
  AC_FC_SRCEXT([f90])
  dnl Yes, we want free form Fortran!
  AC_FC_FREEFORM
fi
AC_LANG_POP

## ----------------------- ##
## Libtool initialisation. ##
## ----------------------- ##
LT_INIT
AC_SUBST([LIBTOOL_DEPS])

## -------- ##
## Outputs. ##
## -------- ##
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
