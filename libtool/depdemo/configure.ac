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

AC_PREREQ(2.50)


## ------------------------ ##
## Autoconf initialisation. ##
## ------------------------ ##
AC_INIT([depdemo], [0.1], [bug-libtool@gnu.org])
AC_CONFIG_SRCDIR([main.c])


## ------------------------ ##
## Automake Initialisation. ##
## ------------------------ ##
AM_INIT_AUTOMAKE(AC_PACKAGE_TARNAME, AC_PACKAGE_VERSION)


## ---------------- ##
## compiler checks. ##
## ---------------- ##
AC_PROG_CC
AC_EXEEXT

## ----------------------- ##
## Libtool initialisation. ##
## ----------------------- ##
AC_LIBTOOL_WIN32_DLL
AM_PROG_LIBTOOL
AC_SUBST([LIBTOOL_DEPS])

## ------------------------------- ##
## depdemo specific configuration. ##
## ------------------------------- ##
if ${CONFIG_SHELL} ./libtool --features | grep "enable static" >/dev/null; then
  STATIC=-static
else
  STATIC=
fi
AC_SUBST([STATIC])

## ---------------------------- ##
## C headers required by cdemo. ##
## ---------------------------- ##
AC_CHECK_HEADERS([math.h])


## ---------------------------- ##
## Libraries required by cdemo. ##
## ---------------------------- ##
AC_CHECK_LIBM
AC_SUBST([LIBM])


## -------- ##
## Outputs. ##
## -------- ##
AC_CONFIG_FILES([Makefile l1/Makefile l2/Makefile l3/Makefile l4/Makefile])
AC_OUTPUT
