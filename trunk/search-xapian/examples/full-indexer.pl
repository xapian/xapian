#!/usr/bin/perl
#
# A sample indexer which demonstrates many of Xapian's commonly used features.
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
use POSIX;

# Constants denoting what we're using the number value slots for.
my $SLOT_DATE = 0;
my $SLOT_DOCNUM = 1;
my $SLOT_TYPE = 2;
my $SLOT_AUTHOR = 3;
my $SLOT_TITLE = 4;

# We want exactly one command line argument.
if (scalar @ARGV != 1) {
    print STDERR "Usage: $0 PATH_TO_DATABASE\n";
    exit 1;
}

my ($database, $indexer);

eval {
    # Open the database for writing.  If it doesn't exist, create it.
    $database = Search::Xapian::WritableDatabase->new(
	    $ARGV[0],
	    DB_CREATE_OR_OPEN);

    # Set up a TermGenerator to index text stemmed with the "english" stemmer.
    $indexer = Search::Xapian::TermGenerator->new();
    my $stemmer = Search::Xapian::Stem->new("english");
    $indexer->set_stemmer($stemmer);
};
if ($@) {
    # Report the exception which we've caught.
    print STDERR "Exception: $@\n";
    exit 1;
}

# Add some documents to the index (in a real indexer, this data would come from
# some external source like a file or a database).
index_document(
    "The Old Man and the Sea",
    "Ernest Hemingway",
    "Santiago goes fishing, without much success.",
    "978-0-684-80122-3",
    "1952-09-01",
    "book"
);
index_document(
    "Star Wars",
    "George Lucas",
    "Luke goes to meet his destiny in the stars.",
    "tt0076759",
    "1977-05-25",
    "film"
);
index_document(
    "Accidental Death of an Anarchist",
    "Dario Fo",
    "An anarchist dies, accidentally!",
    "12345",
    "1970-12-10",
    "play"
);

sub index_document {
    my ($doc_name, $author, $keywords, $doc_number, $date, $type) = @_;

    eval {
	my $doc = Search::Xapian::Document->new();
	$indexer->set_document($doc);

	# Set the document data to the doc_name so we can show it for matches.
	$doc->set_data($doc_name);

	# Index the author to allow fielded free-text searching.
	$indexer->index_text($author, 1, "A");

	# Index the title to allow fielded free-text searching.
	$indexer->index_text($doc_name, 1, "S");

	# Index the title without a prefix too.
	$indexer->index_text($doc_name);

	# Increase the term position so that phrases can't straddle the
	# doc_name and keywords.
	$indexer->increase_termpos();

	# Index the keywords as free-text.
	$indexer->index_text($keywords);

	# Unique ID.
	$doc->add_term("Q" . $doc_number);

	# To allow boolean filtering by type.
	$doc->add_term("XTYPE" . lc $type);

	# To allow date range searching and sorting by date.
	if ($date =~ /^(\d{4})-(\d\d)-(\d\d)$/) {
	    # DateValueRangeProcessor wants values in the form "YYYYMMDD".
	    $doc->add_value($SLOT_DATE, "$1$2$3");
	}

	# To allow sorting by document number.
	$doc->add_value($SLOT_DOCNUM, $doc_number);

	# To allow sorting by document type.
	$doc->add_value($SLOT_TYPE, lc $type);

	# To allow sorting by author.
	$doc->add_value($SLOT_AUTHOR, $author);

	# To allow sorting by title..
	$doc->add_value($SLOT_TITLE, $doc_name);

	# Add the document to the database.
	$database->add_document($doc);
    };
    if ($@) {
	# Report the exception which we've caught.
	print STDERR "Exception: $@\n";
	exit 1;
    }
}
