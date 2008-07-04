## Process this file with autoconf to create configure. -*- autoconf -*-
#
#   Copyright (C) 2001, 2003, 2004, 2005, 2007 Free Software Foundation, Inc.
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
AC_INIT([demo], [1.0], [bug-libtool@gnu.org])
AC_CONFIG_SRCDIR([hello.c])
AC_CONFIG_AUX_DIR([../../libltdl/config])


## ------------------------ ##
## Automake Initialisation. ##
## ------------------------ ##
AM_INIT_AUTOMAKE


## ---------------- ##
## compiler checks. ##
## ---------------- ##
AC_PROG_CC


## ----------------------- ##
## Libtool initialisation. ##
## ----------------------- ##
LT_INIT([dlopen win32-dll])
AC_SUBST([LIBTOOL_DEPS])

STATIC=
test "X$enable_static" = Xyes && STATIC=-static
AC_SUBST([STATIC])

case $lt_cv_sys_global_symbol_pipe in
  ?*) binary_helldl=yes ;;
esac
AM_CONDITIONAL([BINARY_HELLDL], [test "X$binary_helldl" = Xyes])


## --------------------------- ##
## Libraries required by demo. ##
## --------------------------- ##
LT_LIB_M


## -------- ##
## Outputs. ##
## -------- ##
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
