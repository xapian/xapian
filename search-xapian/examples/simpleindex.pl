#!/usr/bin/perl
#
# Index each paragraph of a text file as a Xapian document.
#
# Copyright (C) 2003 James Aylett
# Copyright (C) 2004,2007,2009 Olly Betts
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
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
# USA

use 5.006;
use strict;
use warnings;

use Search::Xapian (':all');

if (scalar @ARGV != 1) {
    print STDERR "Usage: $0 PATH_TO_DATABASE\n";
    exit(1);
}

eval {
    # Open the database for update, creating a new database if necessary.
    my $database = Search::Xapian::WritableDatabase->new($ARGV[0], DB_CREATE_OR_OPEN);

    my $indexer = Search::Xapian::TermGenerator->new();
    my $stemmer = Search::Xapian::Stem->new("english");
    $indexer->set_stemmer($stemmer);

    my $para = '';
    while (my $line = <STDIN>) {
	$line =~ s/\s+$//;
	$line =~ s/^\s+//;
	if ($line eq '') {
	    if ($para ne '') {
		# We've reached the end of a paragraph, so index it.
		my $doc = Search::Xapian::Document->new();
		$doc->set_data($para);

		$indexer->set_document($doc);
		$indexer->index_text($para);

		# Add the document to the database.
		$database->add_document($doc);
		$para = '';
	    }
	} else {
	    if ($para ne '') {
		$para .= ' ';
	    }
	    $para .= $line;
	}
    }
};
if ($@) {
    print STDERR "Exception: $@\n";
    exit(1);
}
