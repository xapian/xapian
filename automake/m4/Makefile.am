## Process this file with automake to create Makefile.in

## Makefile for Automake m4.

## Copyright (C) 1996, 1997, 1998, 1999, 2001, 2002, 2003, 2004, 2006
## Free Software Foundation, Inc.

## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2, or (at your option)
## any later version.

## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.

## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
## 02110-1301, USA.

m4datadir = $(datadir)/aclocal-$(APIVERSION)

dist_m4data_DATA = \
$(top_srcdir)/m4/amversion.m4 \
as.m4 \
auxdir.m4 \
ccstdc.m4 \
cond.m4 \
depend.m4 \
depout.m4 \
dmalloc.m4 \
gcj.m4 \
header.m4 \
init.m4 \
install-sh.m4 \
lead-dot.m4 \
lex.m4 \
lispdir.m4 \
maintainer.m4 \
make.m4 \
minuso.m4 \
missing.m4 \
mkdirp.m4 \
multi.m4 \
obsol-gt.m4 \
obsol-lt.m4 \
obsolete.m4 \
options.m4 \
protos.m4 \
python.m4 \
regex.m4 \
runlog.m4 \
sanity.m4 \
strip.m4 \
substnot.m4 \
tar.m4 \
upc.m4

EXTRA_DIST = dirlist amversion.in

# We build amversion.m4 here, instead of from config.status,
# because config.status is rerun each time one of configure's
# dependencies change and amversion.m4 happens to be a configure
# dependency.  configure and amversion.m4 would be rebuilt in
# loop otherwise.
# Use `$(top_srcdir)/m4' for the benefit of non-GNU makes: this is
# how amversion.m4 appears in our dependencies.
$(top_srcdir)/m4/amversion.m4: $(srcdir)/amversion.in
	sed -e 's,[@]VERSION[@],$(VERSION),g' \
	    -e 's,[@]APIVERSION[@],$(APIVERSION),g' \
	    -e "s,[@]configure_input[@],Generated from amversion.in; do not edit by hand.,g" \
	    $(srcdir)/amversion.in > $@t
	chmod a-w $@t
	mv -f $@t $@
