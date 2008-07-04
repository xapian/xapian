# Customize maint.mk for Autoconf.            -*- Makefile -*-
# Copyright (C) 2003, 2004, 2006, 2008 Free Software Foundation, Inc.

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

# This file is '-include'd into GNUmakefile.

# Build with our own versions of these tools, when possible.
export PATH = $(shell echo "`pwd`/tests:$$PATH")

# Remove the autoreconf-provided INSTALL, so that we regenerate it.
_autoreconf = autoreconf -i -v && rm -f INSTALL

# Version management.
announce_gen   = $(srcdir)/build-aux/announce-gen

# Use alpha.gnu.org for alpha and beta releases.
# Use ftp.gnu.org for major releases.
gnu_ftp_host-alpha = alpha.gnu.org
gnu_ftp_host-beta = alpha.gnu.org
gnu_ftp_host-major = ftp.gnu.org
gnu_rel_host = $(gnu_ftp_host-$(RELEASE_TYPE))

url_dir_list = \
  ftp://$(gnu_rel_host)/gnu/autoconf

# The GnuPG ID of the key used to sign the tarballs.
gpg_key_ID = F4850180

# Files to update automatically.
cvs_executable_files = \
  $(srcdir)/build-aux/config.guess \
  $(srcdir)/build-aux/config.sub \
  $(srcdir)/build-aux/elisp-comp \
  $(srcdir)/build-aux/install-sh \
  $(srcdir)/build-aux/mdate-sh \
  $(srcdir)/build-aux/missing \

cvs_files = $(cvs_executable_files) \
  $(srcdir)/build-aux/texinfo.tex \
  $(srcdir)/doc/fdl.texi \
  $(srcdir)/doc/make-stds.texi \
  $(srcdir)/doc/standards.texi

# Keep executables executable.  Make it robust to parallel makes.
local_updates = executable-update

.PHONY: executable-update
# autom4te-update is defined in Makefile.am.
executable-update: wget-update cvs-update autom4te-update
	chmod a+x $(cvs_executable_files)

# Tests not to run.
local-checks-to-skip ?= \
  changelog-check sc_unmarked_diagnostics
