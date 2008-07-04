# Make Autoconf.

# Copyright (C) 1999, 2000, 2001, 2002, 2003, 2004, 2006, 2007, 2008 Free
# Software Foundation, Inc.

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

# bin/ must be run first, as it builds executables needed for tests.
# autom4te uses autotest.m4f to generate `testsuite', so build tests last.
# Rules in man/ use scripts from both bin/ and tests/, so *it* goes last.
SUBDIRS = bin . lib doc tests man

ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST = ChangeLog.0 ChangeLog.1 ChangeLog.2 \
	     BUGS COPYINGv3 \
	     GNUmakefile maint.mk cfg.mk \
	     build-aux/git-version-gen \
	     build-aux/announce-gen build-aux/gnupload \
	     .prev-version .version

MAINTAINERCLEANFILES = $(srcdir)/INSTALL

## --------- ##
## INSTALL.  ##
## --------- ##

pkgdata_DATA = $(srcdir)/INSTALL
AM_MAKEINFOFLAGS = --no-headers --no-validate --no-split
$(srcdir)/INSTALL: $(top_srcdir)/doc/install.texi
	$(MAKEINFO) $(AM_MAKEINFOFLAGS) $(MAKEINFOFLAGS) -o $@ \
	  $(top_srcdir)/doc/install.texi

## maintainer-check ##
maintainer-check: maintainer-check-tests
maintainer-check-tests: all
	cd tests && $(MAKE) $(AM_MAKEFLAGS) maintainer-check


## ----------------------------------- ##
## Updating Perl files from Automake.  ##
## ----------------------------------- ##

WGET = wget
WGETFLAGS = -C off

## Fetch the latest versions of files we care about.
automake_gitweb = \
  http://git.savannah.gnu.org/gitweb/?p=automake.git;a=blob_plain;hb=HEAD;
autom4te_files = \
  Autom4te/Configure_ac.pm \
  Autom4te/Channels.pm \
  Autom4te/FileUtils.pm \
  Autom4te/Struct.pm \
  Autom4te/XFile.pm

autom4te-update:
	rm -f -r Fetchdir > /dev/null 2>&1
	mkdir -p Fetchdir/Autom4te
	for file in $(autom4te_files); do \
	  infile=`echo $$file | sed 's/Autom4te/Automake/g'`; \
	  $(WGET) $(WGET_FLAGS) \
	    "$(automake_gitweb)f=lib/$$infile" \
	    -O "Fetchdir/$$file" || exit; \
	done
	perl -pi -e 's/Automake::/Autom4te::/g' Fetchdir/Autom4te/*.pm
	for file in $(autom4te_files); do \
	  $(move_if_change) Fetchdir/$$file $(srcdir)/lib/$$file || exit; \
	done

# Version string management.  There are two files to be aware of:
# .tarball-version - present only in a distribution tarball, and not in
#   a checked-out repository.  Created with contents that were learned at
#   the last time autoconf was run, and used by git-version-gen.  Must not
#   be present in either $(srcdir) or $(builddir) for git-version-gen to
#   give accurate answers during normal development with a checked out tree,
#   but must be present in a tarball when there is no version control system.
#   Therefore, it cannot be used in any dependencies.  GNUmakefile has
#   hooks to force a reconfigure at distribution time to get the value
#   correct, without penalizing normal development with extra reconfigures.
# .version - present in a checked-out repository and in a distribution
#   tarball.  At least as current as the most recent .tarball-version
#   creation.  Usable in dependencies, particularly for files that don't
#   want to depend on config.h but do want to track version changes.
BUILT_SOURCES = $(top_srcdir)/.version
$(top_srcdir)/.version:
	echo $(VERSION) > $@-t && mv $@-t $@

# Arrange so that .tarball-version appears only in distribution tarballs,
# never in a checked-out repository.
dist-hook:
	echo $(VERSION) > $(distdir)/.tarball-version

# Arrange to remove the symlink to GNUmakefile in VPATH builds.
# TODO remove this once automake vs. AC_CONFIG_LINKS issue is fixed.
distclean-local:
	if test x"$(VPATH)" != x ; then rm -f GNUmakefile ; fi
