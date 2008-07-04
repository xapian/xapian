## Process this file with automake to create Makefile.in

## Copyright (C) 2001, 2002, 2003, 2004  Free Software Foundation, Inc.

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

SUBDIRS = tests

perllibdir = $(pkgvdatadir)/Automake
dist_perllib_DATA = \
  ChannelDefs.pm \
  Channels.pm \
  Condition.pm \
  Configure_ac.pm \
  DisjConditions.pm \
  FileUtils.pm \
  General.pm \
  Item.pm \
  ItemDef.pm \
  Location.pm \
  Options.pm \
  Rule.pm \
  RuleDef.pm \
  Struct.pm \
  Variable.pm \
  VarDef.pm \
  Version.pm \
  XFile.pm \
  Wrap.pm

nodist_perllib_DATA = \
  Config.pm

CLEANFILES = $(nodist_perllib_DATA)

## We can't use configure to do the substitution here; we must do it
## by hand.  We use a funny notation here to avoid configure
## substitutions in our text.
do_subst = in=`echo $@ | sed 's/\.[^.]*$$//'`; sed \
  -e 's,[@]APIVERSION[@],$(APIVERSION),g' \
  -e 's,[@]PACKAGE[@],$(PACKAGE),g' \
  -e 's,[@]PERL[@],$(PERL),g' \
  -e 's,[@]SHELL[@],$(SHELL),g' \
  -e 's,[@]VERSION[@],$(VERSION),g' \
  -e "s,[@]configure_input[@],Generated from $$in.in; do not edit by hand.,g" \
  -e 's,[@]datadir[@],$(datadir),g'

## This file depends on Makefile so it is rebuilt if $(VERSION),
## $(datadir) or other do_subst'ituted variables change.
## Use chmod a-w to prevent people from editing the wrong file by accident.
Config.pm: Config.in Makefile
	rm -f Config.tmp Config.pm
	$(do_subst) $(srcdir)/Config.in >Config.tmp
	chmod +x Config.tmp
	chmod a-w Config.tmp
	mv -f Config.tmp Config.pm

EXTRA_DIST = Config.in
