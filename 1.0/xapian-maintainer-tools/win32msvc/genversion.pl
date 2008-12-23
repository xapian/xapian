#!/usr/bin/perl -w

# genversion.pl: Generate the version.h file for MSVC builds.
#
# Copyright (C) 2007 Lemur Consulting Ltd
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA

# Simple script to generate version.h from msvc\version.h.in
#
# version.h must be placed in include\xapian\ before compiling xapian-core.
#
# Usage: genversion.pl configure.ac version.h.in version.h

use strict;

my ($confpath, $versionin, $versionout) = @ARGV;

my $major;
my $minor;
my $revision;

open CONFFD, $confpath or die "Can't open \"$confpath\"";
while (<CONFFD>) {
    if (m/AC_INIT\(\[?xapian-.*\]?, \[?([0-9]+)\.([0-9]+)\.([0-9]+).*\)/) {
	$major = $1;
	$minor = $2;
	$revision = $3;
    }
}
close CONFFD;

open VERSIN, $versionin or die "Can't open \"$versionin\"";
open VERSOUT, ">", $versionout or die "Can't open \"$versionout\"";

while (<VERSIN>) {
    s/\@MAJOR_VERSION\@/$major/;
    s/\@MINOR_VERSION\@/$minor/;
    s/\@REVISION\@/$revision/;
    s/\@COMPAT_VERSION\@/$major\.$minor\.$revision/;
    print VERSOUT $_;
}

close VERSIN;
close VERSOUT;
