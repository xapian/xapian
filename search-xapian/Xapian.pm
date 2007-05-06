package Search::Xapian;

use 5.006;
use strict;
use warnings;
use Carp;

use Search::Xapian::Database;
use Search::Xapian::Document;
use Search::Xapian::ESet;
use Search::Xapian::ESetIterator;
use Search::Xapian::MSet;
use Search::Xapian::MSetIterator;
use Search::Xapian::PositionIterator;
use Search::Xapian::PostingIterator;
use Search::Xapian::Query;
use Search::Xapian::QueryParser;
use Search::Xapian::RSet;
use Search::Xapian::TermIterator;
use Search::Xapian::ValueIterator;
use Search::Xapian::WritableDatabase;

use Search::Xapian::BM25Weight;
use Search::Xapian::BoolWeight;
use Search::Xapian::TradWeight;

use Search::Xapian::SimpleStopper;
use Search::Xapian::PerlStopper;

require Exporter;
require DynaLoader;

our @ISA = qw(Exporter DynaLoader);

# Items to export into callers namespace by default. Note: do not export
# names by default without a very good reason. Use EXPORT_OK instead.
# Do not simply export all your public functions/methods/constants.

# This allows declaration	use Search::Xapian ':all';
# If you do not need this, moving things directly into @EXPORT or @EXPORT_OK
# will save memory.
our %EXPORT_TAGS = (
                    'ops' => [ qw(
                                  OP_AND
                                  OP_OR
                                  OP_AND_NOT
                                  OP_XOR
                                  OP_AND_MAYBE
                                  OP_FILTER
                                  OP_NEAR
                                  OP_PHRASE
				  OP_VALUE_RANGE
                                  OP_ELITE_SET
                                 ) ],
                    'db' => [ qw(
                                 DB_OPEN
                                 DB_CREATE
                                 DB_CREATE_OR_OPEN
                                 DB_CREATE_OR_OVERWRITE
                                 ) ],
                    'enq_order' => [ qw(
				 ENQ_DESCENDING
				 ENQ_ASCENDING
				 ENQ_DONT_CARE
				   ) ],
                    'qpflags' => [ qw(
				 FLAG_BOOLEAN
				 FLAG_PHRASE
				 FLAG_LOVEHATE
				 FLAG_BOOLEAN_ANY_CASE
				 FLAG_WILDCARD
				 FLAG_PURE_NOT
				 FLAG_PARTIAL
                                 ) ],
                    'qpstem' => [ qw(
				 STEM_NONE
				 STEM_SOME
				 STEM_ALL
                                 ) ]
                   );
$EXPORT_TAGS{standard} = [ @{ $EXPORT_TAGS{'ops'} },
			   @{ $EXPORT_TAGS{'db'} },
			   @{ $EXPORT_TAGS{'qpflags'} },
			   @{ $EXPORT_TAGS{'qpstem'} } ];
$EXPORT_TAGS{all} = [ @{ $EXPORT_TAGS{'standard'} }, @{ $EXPORT_TAGS{'enq_order'} } ];


our @EXPORT_OK = ( @{ $EXPORT_TAGS{'all'} } );

our @EXPORT = qw( );


our $VERSION = '0.9.99.0';

bootstrap Search::Xapian $VERSION;

# Preloaded methods go here.

our @OP_NAMES;
foreach (@{ $EXPORT_TAGS{'ops'} }) {
  $OP_NAMES[eval $_] = $_;
}

our @DB_NAMES;
foreach (@{ $EXPORT_TAGS{'dbs'} }) {
  $DB_NAMES[eval $_] = $_;
}

our @FLAG_NAMES;
foreach (@{ $EXPORT_TAGS{'qpflags'} }) {
  $FLAG_NAMES[eval $_] = $_;
}

our @STEM_NAMES;
foreach (@{ $EXPORT_TAGS{'qpstem'} }) {
  $STEM_NAMES[eval $_] = $_;
}

1;

__END__


=head1 NAME

Search::Xapian - Perl XS frontend to the Xapian C++ search library.

=head1 SYNOPSIS

  use Search::Xapian;

  my $db = Search::Xapian::Database->new( '[DATABASE DIR]' );
  my $enq = $db->enquire( '[QUERY TERM]' );

  printf "Running query '%s'\n", $enq->get_query()->get_description();

  my @matches = $enq->matches(0, 10);

  print scalar(@matches) . " results found\n";

  foreach my $match ( @matches ) {
    my $doc = $match->get_document();
    printf "ID %d %d%% [ %s ]\n", $match->get_docid(), $match->get_percent(), $doc->get_data();
  }

=head1 DESCRIPTION

This module wraps most methods of most Xapian classes. The missing classes
and methods should be added in the future. It also provides a simplified,
more 'perlish' interface - as demonstrated above.

The Xapian library is evolving very quickly at the time of writing,
hence any documentation placed here would be likely to become out of
date quite rapidly, and I do not have the patience to write some which
could rapidly become redundant.

Apologies to those of you considering using this module. For the time
being, I would suggest garnering what you can from the tests and
examples provided, or reading through the Xapian documentation on
L<http://www.xapian.org/>, notably the API documentation at
L<http://www.xapian.org/docs/apidoc/html/annotated.html>.

If you encounter problems, email either me or preferably the
Xapian-discuss mailing list (which I am on - subscription details can
be found on the Xapian web site L<http://www.xapian.org/lists.php>).

=head2 EXPORT

None by default.

=head1 :db

=over 4

=item DB_OPEN

Open a database, fail if database doesn't exist.

=item DB_CREATE

Create a new database, fail if database exists.

=item DB_CREATE_OR_OPEN

Open an existing database, without destroying data, or create a new
database if one doesn't already exist.

=item DB_CREATE_OR_OVERWRITE

Overwrite database if it exists.

=back

=head1 :ops

=over 4

=item OP_AND

Match if both subqueries are satisfied.

=item OP_OR

Match if either subquery is satisfied.

=item OP_AND_NOT

Match if left but not right subquery is satisfied.

=item OP_XOR

Match if left or right, but not both queries are satisfied.

=item OP_AND_MAYBE

Match if left is satisfied, but use weights from both.

=item OP_FILTER

Like OP_AND, but only weight using the left query.

=item OP_NEAR

Match if the words are near each other. The window should be specified, as
a parameter to C<Search::Xapian::Query::Query>, but it defaults to the 
number of terms in the list.

=item OP_PHRASE

Match as a phrase (All words in order).

=item OP_ELITE_SET

Select an elite set from the subqueries, and perform a query with these combined as an OR query.

=item OP_VALUE_RANGE

Filter by a range test on a document value.

=back

=head1 :qpflags

=over 4

=item FLAG_BOOLEAN

Support AND, OR, etc and bracketted subexpressions.

=item FLAG_LOVEHATE

Support + and -.

=item FLAG_PHRASE

Support quoted phrases.

=item FLAG_BOOLEAN_ANY_CASE

Support AND, OR, etc even if they aren't in ALLCAPS.

=item FLAG_WILDCARD

Support right truncation (e.g. Xap*).

=item FLAG_PURE_NOT

Allow queries such as 'NOT apples'.

These require the use of a list of all documents in the database
which is potentially expensive, so this feature isn't enabled by
default.

=item FLAG_PARTIAL

Enable partial matching.

Partial matching causes the parser to treat the query as a
"partially entered" search.  This will automatically treat the
final word as a wildcarded match, unless it is followed by
whitespace, to produce more stable results from interactive
searches.

=back

=head1 :qpstem

=over 4

=item STEM_ALL

Stem all terms.

=item STEM_NONE

Don't stem any terms.

=item STEM_SOME

Stem some terms, in a manner compatible with Omega (capitalised words aren't
stemmed, and get an 'R' prefix).

=back

=head1 :enq_order

=over 4

=item ENQ_ASCENDING

docids sort in ascending order (default)

=item ENQ_DESCENDING

docids sort in descending order

=item ENQ_DONT_CARE

docids sort in whatever order is most efficient for the backend

=back

=head1 :standard

Standard is db + ops + qpflags + qpstem

=head1 TODO

=over 4

=item Error Handling

Error handling for all methods liable to generate them.

=item Documentation

Add POD documentation for all classes, where possible just adapted from Xapian
docs.

=item Unwrapped classes

The following Xapian classes are not yet wrapped:
Error (and subclasses), ErrorHandler, ExpandDecider (and subclasses),
user-defined weight classes.

=item Unwrapped methods

The following methods are not yet wrapped:
Enquire::get_eset(...) with more than two arguments,
Enquire::register_match_decider(...) with one argument,
Query ctor optional "parameter" parameter,
Remote::open(...),
static Stem::get_available_languages().

We wrap MSet::swap() and MSet::operator[](), but not ESet::swap(),
ESet::operator[]().  Is swap actually useful?  Should we instead tie MSet
and ESet to allow them to just be used as lists?

=back

=head1 CREDITS

Thanks to Tye McQueen E<lt>tye@metronet.comE<gt> for explaining the
finer points of how best to write XS frontends to C++ libraries, James
Aylett E<lt>james@tartarus.orgE<gt> for clarifying the less obvious
aspects of the Xapian API, Tim Brody for patches wrapping ::QueryParser and
::Stopper and especially Olly Betts E<lt>olly@survex.comE<gt> for contributing
advice, bugfixes, and wrapper code for the more obscure classes.

=head1 AUTHOR

Alex Bowley E<lt>kilinrax@cpan.orgE<gt>

=head1 SEE ALSO

L<perl>. L<xapian>.

=cut
