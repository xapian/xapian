#!/usr/bin/perl
#
# Simple command-line search script.
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

# We require at least two command line arguments.
if (scalar @ARGV < 2) {
    print STDERR "Usage: $0 PATH_TO_DATABASE QUERY\n";
    exit(1);
}

eval {
    # Open the database for searching.
    my $database = Search::Xapian::Database->new(shift @ARGV);

    # Start an enquire session.
    my $enquire = Search::Xapian::Enquire->new($database);

    # Combine the rest of the command line arguments with spaces between
    # them, so that simple queries don't have to be quoted at the shell
    # level.
    my $query_string = join ' ', @ARGV;

    # Parse the query string to produce a Xapian::Query object.
    my $qp = Search::Xapian::QueryParser->new();
    my $stemmer = Search::Xapian::Stem->new("english");
    $qp->set_stemmer($stemmer);
    $qp->set_database($database);
    $qp->set_stemming_strategy(STEM_SOME);
    my $query = $qp->parse_query($query_string);
    print "Parsed query is: $query\n";

    # Find the top 10 results for the query.
    $enquire->set_query($query);
    my $mset = $enquire->get_mset(0, 10);

    # Display the results.
    printf "%i results found.\n", $mset->get_matches_estimated();
    printf "Results 1-%i:\n", $mset->size();

    foreach my $m ($mset->items()) {
	printf "%i: %i%% docid=%i [%s]\n", $m->get_rank() + 1, $m->get_percent(), $m->get_docid(), $m->get_document()->get_data();
    }
};
if ($@) {
    print STDERR "Exception: $@\n";
    exit(1);
}
