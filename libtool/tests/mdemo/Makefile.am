## Makefile.am -- Process this file with automake to produce Makefile.in
##
##   Copyright (C) 2003, 2004, 2005, 2007, 2008 Free Software Foundation
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
AM_CPPFLAGS	 = -I$(top_srcdir)/../.. $(INCLTDL)

lib_LTLIBRARIES = libsub.la foo1.la libfoo2.la libmlib.la

foo1_la_SOURCES = foo1.c
foo1_la_LIBADD = $(LIBM) libsub.la
foo1_la_LDFLAGS = -no-undefined -module -avoid-version

libfoo2_la_SOURCES = foo2.c
libfoo2_la_LIBADD = $(LIBM) libsub.la
libfoo2_la_LDFLAGS = -no-undefined -module -export-symbols-regex "libfoo2.*"

libsub_la_SOURCES = sub.c
libsub_la_LDFLAGS = -no-undefined

## Use -export-symbols-regex here explicitly because libltdl marks
## its exported symbols, and we use libltdl as a convenience archive.
## Thus, on w32, auto-exporting is turned off.
libmlib_la_SOURCES = mlib.c
libmlib_la_LIBADD = @LIBLTDL@ "-dlopen" foo1.la "-dlopen" libfoo2.la
libmlib_la_LDFLAGS = -no-undefined -export-symbols-regex ".*"
libmlib_la_DEPENDENCIES = @LIBLTDL@ libsub.la foo1.la libfoo2.la

noinst_HEADERS = foo.h

bin_PROGRAMS = mdemo mdemo_static

## use @LIBLTDL@ because some broken makes do not accept macros in targets
## we can only do this because our LIBLTDL does not contain ${top_builddir}
top_distdir = ../..
@LIBLTDL@: $(top_distdir)/libtool \
    $(top_distdir)/config.h $(srcdir)/$(top_distdir)/libltdl/ltdl.c \
    $(srcdir)/$(top_distdir)/libltdl/ltdl.h
	(cd $(top_distdir); $(MAKE) `echo $(LIBLTDL) | sed 's,.*\.\./libltdl/,libltdl/,g'`)
# Do not error out if toplevel config.h does not exist:
$(top_distdir)/config.h:

# Create a version of mdemo that does dlopen.
mdemo_SOURCES = main.c
mdemo_LDFLAGS = -export-dynamic
## The quotes around -dlopen below fool automake into accepting it
mdemo_LDADD = @LIBLTDL@ libsub.la "-dlopen" self \
		"-dlopen" foo1.la "-dlopen" libfoo2.la
mdemo_DEPENDENCIES = @LIBLTDL@ libsub.la foo1.la libfoo2.la

# Create a statically linked version of mdemo.
mdemo_static_SOURCES = $(mdemo_SOURCES)
mdemo_static_LDFLAGS = $(STATIC) $(mdemo_LDFLAGS)
mdemo_static_LDADD = $(mdemo_LDADD)
mdemo_static_DEPENDENCIES = $(mdemo_DEPENDENCIES)

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
