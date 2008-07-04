## Makefile.am -- Process this file with automake to produce Makefile.in
##
##   Copyright (C) 2003, 2004, 2005 Free Software Foundation
##   Written by Gary V. Vaughan, 2003
##
##   This file is part of GNU Libtool.
##
## GNU Libtool is free software; you can redistribute it and/or
## modify it under the terms of the GNU General Public License as
## published by the Free Software Foundation; either version 2 of
## the License, or (at your option) any later version.
##
## GNU Libtool is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with GNU Libtool; see the file COPYING.  If not, a copy
## can be downloaded from  http://www.gnu.org/licenses/gpl.html,
## or obtained by writing to the Free Software Foundation, Inc.,
## 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
#####

AUTOMAKE_OPTIONS = no-dependencies foreign
ACLOCAL_AMFLAGS  = -I ../../libltdl/m4
AM_CPPFLAGS	 = -I$(top_srcdir)/../..

SUBDIRS = l1 l2 l3 l4

EXTRA_DIST = sysdep.h

bin_PROGRAMS = depdemo depdemo_static

depdemo_SOURCES = main.c
depdemo_LDADD = $(top_builddir)/l1/libl1.la $(top_builddir)/l2/libl2.la \
		$(top_builddir)/l4/libl4.la
depdemo_DEPENDENCIES = $(top_builddir)/l1/libl1.la \
		$(top_builddir)/l2/libl2.la $(top_builddir)/l4/libl4.la

depdemo_static_SOURCES = $(depdemo_SOURCES)
depdemo_static_LDADD = $(depdemo_LDADD)
depdemo_static_DEPENDENCIES = $(depdemo_DEPENDENCIES)
depdemo_static_LDFLAGS = $(STATIC)

libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status --recheck

# Workaround a bug in Autoconf-2.61 and earlier that don't clean up
# file droppings left by many compilers:
distclean-local:
	ac_files="a.out.* a.exe.* a_out.exe.* b.out.* conftest.*"; \
	for ac_file in $$ac_files; do \
	  case $$ac_file in \
	    *.xcoff | *.tds | *.d | *.pdb | *.xSYM | *.bb | *.bbg | *.map | *.inf | *.dSYM ) rm -rf $$ac_file ;; \
	  esac; \
	done
