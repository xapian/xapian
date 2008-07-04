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

# Build a libtool library, libhello.la for installation in libdir.
lib_LTLIBRARIES = libhello.la
libhello_la_SOURCES = longer_file_name_hello.c longer_file_name_foo.c longer_file_name_foo2.c
libhello_la_LIBADD = $(LIBM)
libhello_la_LDFLAGS = -no-undefined -version-info 3:12:1

include_HEADERS = foo.h

if BINARY_HELLDL
BUILD_helldl = helldl
else
BUILD_helldl =
endif

bin_PROGRAMS = hell hell_static $(BUILD_helldl)

# Build hell from longer_file_name_main.c and libhello.la
hell_SOURCES = longer_file_name_main.c
hell_LDADD = libhello.la

# Create a statically linked version of hell.
hell_static_SOURCES = longer_file_name_main.c
hell_static_LDADD = libhello.la
hell_static_LDFLAGS = $(STATIC)

if BINARY_HELLDL

# Create a version of hell that does a preloaded dlopen.
helldl_SOURCES = longer_file_name_dlmain.c
helldl_LDFLAGS = -export-dynamic -dlpreopen libhello.la
helldl_DEPENDENCIES = libhello.la

else

bin_SCRIPTS = helldl
# create a script that says that -dlopen is not supported
helldl helldl$(EXEEXT):
	rm -f $@
	echo '#! /bin/sh' > $@
	echo '-dlopen is unsupported' >> $@
	chmod +x $@
endif

libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status --recheck

#----------------------------------------------------------------------
# The following declarations are only used in the testsuite:
#----------------------------------------------------------------------

hardcode_tests = hc-direct hc-libflag hc-libpath hc-minusL
CLEANFILES = $(hardcode_tests)

# Unfortunately, in order to test libtool thoroughly, we need access
# to its private directory.
objdir = `$(LIBTOOL) --config | sed -n -e 's/^objdir=\(.*\)$$/\1/p'`

# Test programs to see what gets hardcoded.
.PHONY: hardcode
hardcode: $(hardcode_tests)
SET_HARDCODE_FLAGS = \
  eval `$(LIBTOOL) --config | sed -n -e '/^hardcode_.*=/p; /^wl=/p'`
hc-direct: $(hell_OBJECTS) $(hell_DEPENDENCIES) $(libdir)/libhello.la
	@rm -f hc-direct
	@echo "You may ignore any linking errors from the following command:"
	@shlib=./$(objdir)/libhello.a; \
	  eval "`grep '^library_names' libhello.la`"; \
	  for lib in $$library_names; do \
	    shlib="./$(objdir)/$$lib"; \
	  done; \
	  $(SET_HARDCODE_FLAGS); \
	  libdir=$(libdir); \
	  flag=`eval echo \"$$hardcode_libdir_flag_spec\"`; \
	  echo "$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(hell_OBJECTS) $$shlib $(LIBS) $(LIBM) $$flag || echo unsupported > $@"; \
	  eval "$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(hell_OBJECTS) $$shlib $(LIBS) $(LIBM) $$flag || echo unsupported > $@"

hc-libflag: $(hell_OBJECTS) $(hell_DEPENDENCIES) $(libdir)/libhello.la
	@$(SET_HARDCODE_FLAGS); \
	  libdir=`pwd`/$(objdir); \
	  flag=`eval echo \"$$hardcode_libdir_flag_spec\"`; \
	  if test -z "$$flag"; then \
	    echo "echo unsupported > $@"; \
	    echo unsupported > $@ || status="$$?"; \
	  else \
	    echo "$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(hell_OBJECTS) $$flag -L$(libdir) -lhello $(LIBS) $(LIBM)"; \
	    $(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(hell_OBJECTS) $$flag -L$(libdir) -lhello $(LIBS) $(LIBM) || status="$$?"; \
	  fi; \
	  exit $$status

hc-libpath: $(hell_OBJECTS) $(hell_DEPENDENCIES) $(libdir)/libhello.la
	@rm -f hc-libpath
	@echo "You may ignore any linking errors from the following command:"
	@$(SET_HARDCODE_FLAGS); \
	  eval `$(LIBTOOL) --config | grep '^shlibpath_var='`; \
	  libdir=$(libdir); \
	  flag=`eval echo \"$$hardcode_libdir_flag_spec\"`; \
	  echo "$$shlibpath_var=./$(objdir) $(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(hell_OBJECTS) -lhello $(LIBS) $(LIBM) $$flag || echo unsupported > $@"; \
	  eval "$$shlibpath_var=./$(objdir) $(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(hell_OBJECTS) -lhello $(LIBS) $(LIBM) $$flag || echo unsupported > $@"

hc-minusL: $(hell_OBJECTS) $(hell_DEPENDENCIES)
	@rm -f hc-minusL
	@$(SET_HARDCODE_FLAGS); \
	  libdir=$(libdir); \
	  flag=`eval echo \"$$hardcode_libdir_flag_spec\"`; \
	  echo "$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(hell_OBJECTS) -L./$(objdir) -lhello $(LIBS) $(LIBM) $$flag || echo unsupported > $@"; \
	  eval "$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(hell_OBJECTS) -L./$(objdir) -lhello $(LIBS) $(LIBM) $$flag || echo unsupported > $@"

# This is one of the essential tests for deplibs_check_method=pass_all.
# If this one passes with pass_all, it is likely that pass_all works
EXTRA_LIBRARIES = libhell0.a
libhell0_a_SOURCES =
libhell0_a_LIBADD = longer_file_name_hello.$(OBJEXT) longer_file_name_foo.$(OBJEXT)
EXTRA_LTLIBRARIES = libhell1.la libhell2.la
libhell1_la_SOURCES = longer_file_name_hell1.c
libhell1_la_LIBADD = -L. -lhell0
libhell1_la_LDFLAGS = -no-undefined -rpath $(libdir)
libhell1_la_DEPENDENCIES = libhell0.a
libhell2_la_SOURCES = longer_file_name_hell2.c
libhell2_la_LIBADD = -L. -lhell0
libhell2_la_LDFLAGS = -no-undefined -rpath $(libdir)
libhell2_la_DEPENDENCIES = libhell0.a
EXTRA_PROGRAMS = hell0
hell0_SOURCES = longer_file_name_main.c
hell0_LDADD = libhell1.la libhell2.la $(LIBM)

# 'hell0' in EXTRA_PROGRAMS gets translated to 'hell0.exe'; but we
# must explicitly list the wrapper script 'hell0'.  (bin_PROGRAMS
# are handled seamlessly by automake rules; the extra step is only
# necessary for EXTRA_PROGRAMS)
CLEANFILES += $(EXTRA_LIBRARIES) $(EXTRA_LTLIBRARIES) $(EXTRA_PROGRAMS) hell0

deplibs-check: hell0

# Workaround a bug in Autoconf-2.61 and earlier that don't clean up
# file droppings left by many compilers:
distclean-local:
	ac_files="a.out.* a.exe.* a_out.exe.* b.out.* conftest.*"; \
	for ac_file in $$ac_files; do \
	  case $$ac_file in \
	    *.xcoff | *.tds | *.d | *.pdb | *.xSYM | *.bb | *.bbg | *.map | *.inf | *.dSYM ) rm -rf $$ac_file ;; \
	  esac; \
	done
