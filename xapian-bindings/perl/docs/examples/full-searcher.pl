#!/usr/bin/perl
#
# A sample search program which demonstrates many of Xapian's commonly used
# features.
#
# Copyright (C) 2009 Olly Betts
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA

use 5.006;
use strict;
use warnings;

use Search::Xapian (':all');

# Constants denoting what we're using the number value slots for.
my $SLOT_DATE = 0;
my $SLOT_DOCNUM = 1;
my $SLOT_TYPE = 2;
my $SLOT_AUTHOR = 3;
my $SLOT_TITLE = 4;

# We need at least three command line arguments.
if (scalar @ARGV < 3) {
    print STDERR "Usage: $0 PATH_TO_DATABASE[...] -- [-sSORTBY] QUERY[...]\n";
    print STDERR "where SORTBY can be date, id, type, author, or title.\n";
    print STDERR "\n";
    print STDERR "Search syntax supported:\n";
    print STDERR "  Boolean filters: type:book id:tt0076759\n";
    print STDERR "  Date ranges: 25/12/1970..31/12/1979\n";
    print STDERR "  Boolean operators: AND OR NOT\n";
    print STDERR "  Free text fields: author:John title:\"star wars\"\n";
    print STDERR "  Phrases: \"accidental death\"\n";
    exit 1;
}

eval {
    # Open the database(s) for searching.
    my $database = Search::Xapian::Database->new(shift @ARGV);
    while (scalar @ARGV && $ARGV[0] ne '--') {
	# Xapian can transparently search several databases together.
	my $extra_db = Search::Xapian::Database->new(shift @ARGV);
	$database->add_database($extra_db);
    }
    shift @ARGV;

    # Default is sort by relevance.
    my $sort_by;
    if (scalar @ARGV && $ARGV[0] =~ /^-s(\w+)$/) {
	if ($1 eq 'date') {
	    $sort_by = 0;
	} elsif ($1 eq 'id') {
	    $sort_by = 1;
	} elsif ($1 eq 'type') {
	    $sort_by = 2;
	} elsif ($1 eq 'author') {
	    $sort_by = 3;
	} elsif ($1 eq 'title') {
	    $sort_by = 4;
	} else {
	    print STDERR "Bad option '-s$1'.\n";
	    exit 1;
	}
	shift @ARGV;
    }

    # Start an enquire session.
    my $enquire = Search::Xapian::Enquire->new($database);

    # Combine the remaining command line arguments with a space between each.
    # This means that simple queries without shell metacharacters in don't
    # have to be quoted just to appear as a single argument to the shell.
    my $query_string = join ' ', @ARGV;

    # Set up the QueryParser how we want.
    my $qp = Search::Xapian::QueryParser->new();
    $qp->set_database($database);
    $qp->set_stemmer(Search::Xapian::Stem->new("english"));
    $qp->set_stemming_strategy(STEM_SOME);

    # Prefixes for free-text fields.
    $qp->add_prefix('title', 'S');
    $qp->add_prefix('author', 'A');

    # Prefixes for boolean filters.
    $qp->add_boolean_prefix('type', 'XTYPE');
    $qp->add_boolean_prefix('id', 'Q');

    # Second argument of 1 means "prefer mm/dd/yyyy".
    # Third argument means that two digit years < 20 are 20xx; >= 29 are 19xx.
    my $vrpdate = new Search::Xapian::DateValueRangeProcessor($SLOT_DATE, 1,
							      1920);
    $qp->add_valuerangeprocessor($vrpdate);

    # Parse the query string and return a Xapian::Query object.
    my $query = $qp->parse_query(
	    $query_string,
	    FLAG_PHRASE|FLAG_BOOLEAN|FLAG_LOVEHATE|FLAG_WILDCARD
	    );

    print "Internal view of parsed query is:\n$query\n\n";

    $enquire->set_query($query);
    if (defined $sort_by) {
	$enquire->set_sort_by_value($sort_by, 0);
    }

    # Return the top 10 results for the query.
    my $mset = $enquire->get_mset(0, 10);

    my $msize = $mset->size();
    if ($msize == 0) {
	print "No matching documents found.\n";
	exit 0;
    }

    # Display the results.
    if ($mset->get_matches_lower_bound() != $mset->get_matches_upper_bound()) {
	print "About ";
    }
    printf "%u matching documents were found.\n",
	   $mset->get_matches_estimated();
    print "Results 1-$msize:\n";

    foreach my $m ($mset->items()) {
	printf "#%u: Score %u%%: %s\n",
	      $m->get_rank() + 1,
	      $m->get_percent(),
	      $m->get_document()->get_data();
    }
};
if ($@) {
    # Report the exception which we've caught.
    print STDERR "Exception: $@\n";
    exit 1;
}
