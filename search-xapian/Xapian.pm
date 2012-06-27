package Search::Xapian;

use 5.006;
use strict;
use warnings;

our $VERSION = '1.2.12.0';

use Exporter 'import';

use Search::Xapian::Database;
use Search::Xapian::Document;
use Search::Xapian::ESet;
use Search::Xapian::ESetIterator;
use Search::Xapian::Error;
use Search::Xapian::MSet;
use Search::Xapian::MSetIterator;
use Search::Xapian::MultiValueSorter;
use Search::Xapian::PositionIterator;
use Search::Xapian::PostingIterator;
use Search::Xapian::Query;
use Search::Xapian::QueryParser;
use Search::Xapian::RSet;
use Search::Xapian::Stem;
use Search::Xapian::TermGenerator;
use Search::Xapian::TermIterator;
use Search::Xapian::ValueIterator;
use Search::Xapian::WritableDatabase;

use Search::Xapian::BM25Weight;
use Search::Xapian::BoolWeight;
use Search::Xapian::TradWeight;

use Search::Xapian::SimpleStopper;
use Search::Xapian::PerlStopper;

require DynaLoader;

our @ISA = qw(DynaLoader);

# We need to use the RTLD_GLOBAL flag to dlopen() so that other C++
# modules that link against libxapian.so get the *same* value for all the
# weak symbols (eg, the exception classes)
sub dl_load_flags { 0x01 }

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
				  OP_SCALE_WEIGHT
                                  OP_ELITE_SET
				  OP_VALUE_GE
				  OP_VALUE_LE
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
				 FLAG_SPELLING_CORRECTION
				 FLAG_SYNONYM
				 FLAG_AUTO_SYNONYMS
				 FLAG_AUTO_MULTIWORD_SYNONYMS
				 FLAG_DEFAULT
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


# Names which can be exported.
our @EXPORT_OK = ( @{ $EXPORT_TAGS{'all'} } );

# Don't export any names by default.
our @EXPORT = qw( );

bootstrap Search::Xapian $VERSION;

# Preloaded methods go here.

our @OP_NAMES;
foreach (@{ $EXPORT_TAGS{'ops'} }) {
  $OP_NAMES[eval $_] = $_;
}

our @DB_NAMES;
foreach (@{ $EXPORT_TAGS{'db'} }) {
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
more 'perlish' interface to some common operations, as demonstrated above.

There are some gaps in the POD documentation for wrapped classes, but you
can read the Xapian C++ API documentation at
L<http://xapian.org/docs/apidoc/html/annotated.html> for details of
these.  Alternatively, take a look at the code in the examples and tests.

If you want to use Search::Xapian and the threads module together, make
sure you're using Search::Xapian >= 1.0.4.0 and Perl >= 5.8.7.  As of 1.0.4.0,
Search::Xapian uses CLONE_SKIP to make sure that the perl wrapper objects
aren't copied to new threads - without this the underlying C++ objects can get
destroyed more than once.

If you encounter problems, or have any comments, suggestions, patches, etc
please email the Xapian-discuss mailing list (details of which can be found at
L<http://xapian.org/lists>).

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

=item FLAG_DEFAULT

This gives the QueryParser default flag settings, allowing you to easily add
flags to the default ones.

=item FLAG_BOOLEAN

Support AND, OR, etc and bracketed subexpressions.

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

=item FLAG_SPELLING_CORRECTION

=item FLAG_SYNONYM

=item FLAG_AUTO_SYNONYMS

=item FLAG_AUTO_MULTIWORD_SYNONYMS

=back

=head1 :qpstem

=over 4

=item STEM_ALL

Stem all terms.

=item STEM_NONE

Don't stem any terms.

=item STEM_SOME

Stem some terms, in a manner compatible with Omega (capitalised words and those
in phrases aren't stemmed).

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

=head1 Version functions

=over 4

=item major_version

Returns the major version of the Xapian C++ library being used.  E.g. for
Xapian 1.0.9 this would return 1.

=item minor_version

Returns the minor version of the Xapian C++ library being used.  E.g. for
Xapian 1.0.9 this would return 0.

=item revision

Returns the revision of the Xapian C++ library being used.  E.g. for
Xapian 1.0.9 this would return 9.  In a stable release series, Xapian libraries
with the same minor and major versions are usually ABI compatible, so this
often won't match the third component of $Search::Xapian::VERSION (which is the
version of the Search::Xapian XS wrappers).

=back

=head1 Numeric encoding functions

=over 4

=item sortable_serialise NUMBER

Convert a floating point number to a string, preserving sort order.

This method converts a floating point number to a string, suitable for
using as a value for numeric range restriction, or for use as a sort
key.

The conversion is platform independent.

The conversion attempts to ensure that, for any pair of values supplied
to the conversion algorithm, the result of comparing the original
values (with a numeric comparison operator) will be the same as the
result of comparing the resulting values (with a string comparison
operator).  On platforms which represent doubles with the precisions
specified by IEEE_754, this will be the case: if the representation of
doubles is more precise, it is possible that two very close doubles
will be mapped to the same string, so will compare equal.

Note also that both zero and -zero will be converted to the same
representation: since these compare equal, this satisfies the
comparison constraint, but it's worth knowing this if you wish to use
the encoding in some situation where this distinction matters.

Handling of NaN isn't (currently) guaranteed to be sensible.

=item sortable_unserialise SERIALISED_NUMBER

Convert a string encoded using sortable_serialise back to a floating
point number.

This expects the input to be a string produced by sortable_serialise().
If the input is not such a string, the value returned is undefined (but
no error will be thrown).

The result of the conversion will be exactly the value which was
supplied to sortable_serialise() when making the string on platforms
which represent doubles with the precisions specified by IEEE_754, but
may be a different (nearby) value on other platforms.

=back

=head1 TODO

=over 4

=item Error Handling

Error handling for all methods liable to generate them.

=item Documentation

Add POD documentation for all classes, where possible just adapted from Xapian
docs.

=item Unwrapped classes

The following Xapian classes are not yet wrapped:
Error (and subclasses), ErrorHandler, standard ExpandDecider subclasses
(user-defined ones works),
user-defined weight classes.

We don't yet wrap Xapian::Query::MatchAll, Xapian::Query::MatchNothing,
or Xapian::BAD_VALUENO.

=item Unwrapped methods

The following methods are not yet wrapped:
Enquire::get_eset(...) with more than two arguments,
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

Please report any bugs/suggestions to E<lt>xapian-discuss@lists.xapian.orgE<gt>
or use the Xapian bug tracker L<http://xapian.org/bugs>.  Please do
NOT use the CPAN bug tracker or mail any of the authors individually.

=head1 SEE ALSO

L<Search::Xapian::BM25Weight>,
L<Search::Xapian::BoolWeight>,
L<Search::Xapian::Database>,
L<Search::Xapian::Document>,
L<Search::Xapian::Enquire>,
L<Search::Xapian::MultiValueSorter>,
L<Search::Xapian::PositionIterator>,
L<Search::Xapian::PostingIterator>,
L<Search::Xapian::QueryParser>,
L<Search::Xapian::Stem>,
L<Search::Xapian::TermGenerator>,
L<Search::Xapian::TermIterator>,
L<Search::Xapian::TradWeight>,
L<Search::Xapian::ValueIterator>,
L<Search::Xapian::Weight>,
L<Search::Xapian::WritableDatabase>,
and
L<http://xapian.org/>.

=cut
