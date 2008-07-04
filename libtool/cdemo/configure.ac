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
AC_INIT([cdemo], [0.1], [bug-libtool@gnu.org])
AC_CONFIG_SRCDIR([main.c])


## ------------------------ ##
## Automake Initialisation. ##
## ------------------------ ##
AM_INIT_AUTOMAKE(AC_PACKAGE_TARNAME, AC_PACKAGE_VERSION)


## ------------------ ##
## C compiler checks. ##
## ------------------ ##
AC_PROG_CC


## ----------------------- ##
## Libtool initialisation. ##
## ----------------------- ##
AM_PROG_LIBTOOL
AC_SUBST([LIBTOOL_DEPS])


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
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
