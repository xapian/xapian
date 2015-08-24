#!/usr/bin/perl
#
# Simple example script demonstrating query expansion.
#
# Copyright (C) 2003 James Aylett
# Copyright (C) 2004,2006,2007,2009 Olly Betts
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
    print STDERR "Usage: $0 PATH_TO_DATABASE QUERY [-- [DOCID...]]\n";
    exit(1);
}

eval {
    # Open the database for searching.
    my $database = Search::Xapian::Database->new(shift @ARGV);

    # Start an enquire session.
    my $enquire = Search::Xapian::Enquire->new($database);

    # Combine command line arguments up to "--" with spaces between
    # them, so that simple queries don't have to be quoted at the shell
    # level.
    my $sep_index = 0;
    while (exists $ARGV[$sep_index] && $ARGV[$sep_index] ne '--') {
	$sep_index++;
    }

    my $query_string = join ' ', @ARGV[0 .. $sep_index - 1];

    # Create an RSet with the listed docids in.
    my $reldocs = Search::Xapian::RSet->new();
    foreach my $did (@ARGV[$sep_index + 1 .. $#ARGV]) {
	$reldocs->add_document($did);
    }

    # Parse the query string to produce a Xapian::Query object.
    my $qp = Search::Xapian::QueryParser->new();
    my $stemmer = Search::Xapian::Stem->new("english");
    $qp->set_stemmer($stemmer);
    $qp->set_database($database);
    $qp->set_stemming_strategy(STEM_SOME);
    my $query = $qp->parse_query($query_string);

    my $mset;
    if (!$query->empty()) {
	print "Parsed query is: $query\n";

	# Find the top 10 results for the query.
	$enquire->set_query($query);
	$mset = $enquire->get_mset(0, 10, $reldocs);

	# Display the results.
	printf "%i results found.\n", $mset->get_matches_estimated();
	printf "Results 1-%i:\n", $mset->size();

	foreach my $m ($mset->items()) {
	    printf "%i: %i%% docid=%i [%s]\n", $m->get_rank() + 1, $m->get_percent(), $m->get_docid(), $m->get_document()->get_data();
	}
    }

    # Put the top 5 (at most) docs into the rset if rset is empty
    if ($reldocs->empty() && defined $mset) {
	my $last = $mset->size() - 1;
	if ($last > 4) {
	   $last = 4;
	}
	foreach my $m (($mset->items())[0..$last]) {
	    $reldocs->add_document($m->get_docid());
	}
    }

    # Get the suggested expand terms
    my $eset = $enquire->get_eset(10, $reldocs);
    printf "%i suggested additional terms\n", $eset->size();
    for my $k ($eset->items()) {
	printf "%s: %f\n", $k->get_termname(), $k->get_weight();
    }
};
if ($@) {
    print STDERR "Exception: $@\n";
    exit(1);
}
