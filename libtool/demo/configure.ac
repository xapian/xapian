## Process this file with autoconf to create configure. -*- autoconf -*-
# Copyright 2001  Free Software Foundation, Inc.
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

AC_PREREQ(2.59)


## ------------------------ ##
## Autoconf initialisation. ##
## ------------------------ ##
AC_INIT([demo],[1.0],[bug-libtool@gnu.org])
AC_CONFIG_SRCDIR([hello.c])


## ------------------------ ##
## Automake Initialisation. ##
## ------------------------ ##
AM_INIT_AUTOMAKE(AC_PACKAGE_TARNAME, AC_PACKAGE_VERSION)


## ---------------- ##
## compiler checks. ##
## ---------------- ##
AC_PROG_CC
AC_C_CONST
AC_EXEEXT


## ----------------------- ##
## Libtool initialisation. ##
## ----------------------- ##
AC_LIBTOOL_DLOPEN
AC_LIBTOOL_WIN32_DLL
AM_PROG_LIBTOOL
AC_SUBST([LIBTOOL_DEPS])

if ./libtool --features | grep '^enable static libraries$' > /dev/null 2>&1; then
  STATIC=-static
else
  STATIC=
fi
AC_SUBST([STATIC])

AM_CONDITIONAL([BINARY_HELLDL],
    [grep '^global_symbol_pipe=..*$' ./libtool >/dev/null])


## ---------------------------- ##
## C headers required by cdemo. ##
## ---------------------------- ##
AC_CHECK_HEADERS([string.h math.h])


## --------------------------- ##
## Libraries required by demo. ##
## --------------------------- ##
AC_CHECK_LIBM
AC_SUBST([LIBM])


## -------- ##
## Outputs. ##
## -------- ##
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
