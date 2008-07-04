#                                                       -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

# Copyright (C) 1992, 1993, 1994, 1995, 1999, 2000, 2001, 2002, 2003,
# 2004, 2005, 2006, 2007, 2008 Free Software Foundation, Inc.

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

# We need AC_CONFIG_TESTDIR, AC_PROG_SED, AC_PROG_GREP.
AC_PREREQ([2.60])

AC_INIT([GNU Autoconf],
	m4_esyscmd([build-aux/git-version-gen .tarball-version]),
	[bug-autoconf@gnu.org])
AC_CONFIG_SRCDIR([ChangeLog])

AC_CONFIG_AUX_DIR([build-aux])

AM_INIT_AUTOMAKE([1.10 dist-bzip2 readme-alpha no-texinfo.tex])

# We use `/bin/sh -n script' to check that there are no syntax errors
# in the scripts.  Although incredible, there are /bin/sh that go into
# endless loops with `-n', e.g., SunOS's:
#
#   $ uname -a
#   SunOS ondine 4.1.3 2 sun4m unknown
#   $ cat endless.sh
#   while false
#   do
#     :
#   done
#   exit 0
#   $ time sh endless.sh
#   sh endless.sh  0,02s user 0,03s system 78% cpu 0,064 total
#   $ time sh -nx endless.sh
#   ^Csh -nx endless.sh  3,67s user 0,03s system 63% cpu 5,868 total
#
# Also, some implementations of /bin/sh (e.g., Solaris 8) are soooo slow
# that they are unusable on large scripts like our testsuite.

# So before using `/bin/sh -n' to check our scripts, we first check
# that `/bin/sh -n' is known to not have these problems.

AC_CACHE_CHECK([whether /bin/sh -n is known to work], [ac_cv_sh_n_works],
[if (
    unset BASH_VERSION ZSH_VERSION
    /bin/sh -c '
      test -n "${BASH_VERSION+set}" || # Bash
      test -n "${KSH_VERSION+set}" || # pdksh
      test -n "${ZSH_VERSION+set}" || # zsh
      test -n "${.sh.version}" # ksh93; put this last since its syntax is dodgy
    '
  ) 2>/dev/null
then ac_cv_sh_n_works=yes
else ac_cv_sh_n_works=no
fi
])
AC_SUBST([ac_cv_sh_n_works])

# Initialize the test suite.
AC_CONFIG_TESTDIR([tests])
AC_CONFIG_FILES([tests/Makefile tests/atlocal])
AC_PATH_PROG([EXPR], [expr])


## ---- ##
## M4.  ##
## ---- ##

# We use an absolute name for GNU m4 so even if users have another m4 first in
# their path, the installer can configure with a path that has GNU m4
# on it and get that path embedded in the installed autoconf and
# autoheader scripts.
AC_PROG_GNU_M4

## ----------- ##
## Man pages.  ##
## ----------- ##

AC_CONFIG_FILES([man/Makefile])
AM_MISSING_PROG([HELP2MAN], [help2man])


## ------ ##
## Perl.  ##
## ------ ##

# We use an absolute name for perl so the #! line in autoscan will work.
AC_PATH_PROG([PERL], [perl], [no])
AC_SUBST([PERL])dnl
if test "$PERL" = no; then
  AC_MSG_ERROR([perl is not found])
fi
$PERL -e 'require 5.005_03;' || {
   AC_MSG_ERROR([Perl 5.005_03 or better is required])
}


## ------- ##
## Emacs.  ##
## ------- ##

AC_CONFIG_FILES([lib/emacs/Makefile])
TEST_EMACS=$EMACS
test x"$TEST_EMACS" = xt && TEST_EMACS=
AC_CHECK_PROGS([TEST_EMACS], [emacs xemacs], [no])

# autoconf-mode.el and autotest-mode.el do not work with older version of
# Emacs (i.e. 18.x.x).  During byte-compilation, Emacs complains:
# "Variable (broken nil) seen on pass 2 of byte compiler but not pass 1"
# We detect this problem here.
AS_IF([test "$TEST_EMACS" != no],
  [AC_CACHE_CHECK([whether $TEST_EMACS is sufficiently recent],
    [ac_cv_prog_emacs_ok],
    [# Note: The quoted "EOF" is intentional.  It protects the ` in the text.
    cat >conftest.el << "EOF"
      (defvar broken)
      (defun broken-mode () (setq broken-defaults `(broken nil)))
EOF
    AS_IF([AC_RUN_LOG(
      [$TEST_EMACS -batch -q -f batch-byte-compile conftest.el 1>&2])],
      [ac_cv_prog_emacs_ok=yes], [ac_cv_prog_emacs_ok=no])
    rm -f conftest.el conftest.elc])
  AS_IF([test $ac_cv_prog_emacs_ok = no], [TEST_EMACS=no])])

AC_SUBST([EMACS], [$TEST_EMACS])

AM_PATH_LISPDIR


## ------------ ##
## Grep & sed.  ##
## ------------ ##
AC_PROG_GREP
AC_PROG_EGREP
AC_PROG_SED


## ------------ ##
## Conclusion.  ##
## ------------ ##

dnl Allow maintainer rules under GNU make even in VPATH builds.  This does
dnl not work in autoconf 2.61 or earlier, but we don't want to require
dnl unreleased autoconf during bootstrap, hence the version test.
dnl TODO remove the version check once we depend on autoconf 2.62.
dnl Meanwhile, we must use a shell variable so that we bypass automake's
dnl attempts to remove the sole copy of GNUmakefile in a non-VPATH build
dnl during 'make distclean'.
dnl TODO avoid the shell variable once automake is fixed.
GNUmakefile=GNUmakefile
m4_if(m4_version_compare([2.61a.100],
	m4_defn([m4_PACKAGE_VERSION])), [1], [],
      [AC_CONFIG_LINKS([$GNUmakefile:$GNUmakefile], [],
	[GNUmakefile=$GNUmakefile])])

AC_CONFIG_FILES([Makefile doc/Makefile
		 lib/Makefile lib/Autom4te/Makefile lib/autoscan/Makefile
		 lib/m4sugar/Makefile
		 lib/autoconf/Makefile lib/autotest/Makefile
		 bin/Makefile])

AC_OUTPUT
# Report the state of this version of Autoconf if this is a beta.
m4_bmatch(m4_defn([AC_PACKAGE_VERSION]),  [[a-z]],
[    cat <<EOF

You are about to use an experimental version of Autoconf.  Be sure to
read the relevant mailing lists, most importantly <autoconf@gnu.org>.

Below you will find information on the status of this version of Autoconf.


EOF
    sed -n '/^\* Status/,$p' $srcdir/BUGS
])dnl
