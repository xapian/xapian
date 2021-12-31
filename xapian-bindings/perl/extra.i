%{
/* perl/extra.i: custom Perl code for xapian-bindings
 *
 * Based on the perl XS wrapper files.
 *
 * Copyright (C) 2009 Kosei Moriyama
 * Copyright (C) 2011,2012,2013,2015,2016,2018,2019,2020 Olly Betts
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */
%}

/* Perl code */
%perlcode {
package Xapian;

our $VERSION = PERL_XAPIAN_VERSION;
};
%perlcode %{
# We need to use the RTLD_GLOBAL flag to dlopen() so that other C++
# modules that link against libxapian.so get the *same* value for all the
# weak symbols (eg, the exception classes)
sub dl_load_flags { 0x01 }

# Items to export into caller's namespace by default. Note: do not export
# names by default without a very good reason. Use EXPORT_OK instead.
# Do not simply export all your public functions/methods/constants.

# This allows declaration use Xapian ':all';
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
				 FLAG_ACCUMULATE
				 FLAG_BOOLEAN
				 FLAG_FUZZY
				 FLAG_NO_POSITIONS
				 FLAG_PHRASE
				 FLAG_LOVEHATE
				 FLAG_BOOLEAN_ANY_CASE
				 FLAG_WILDCARD
				 FLAG_WILDCARD_GLOB
				 FLAG_WILDCARD_MULTI
				 FLAG_WILDCARD_SINGLE
				 FLAG_PURE_NOT
				 FLAG_PARTIAL
				 FLAG_SPELLING_CORRECTION
				 FLAG_SYNONYM
				 FLAG_AUTO_SYNONYMS
				 FLAG_AUTO_MULTIWORD_SYNONYMS
				 FLAG_CJK_NGRAM
				 FLAG_CJK_WORDS
				 FLAG_DEFAULT
				 ) ],
		    'qpstem' => [ qw(
				 STEM_NONE
				 STEM_SOME
				 STEM_SOME_FULL_POS
				 STEM_ALL
				 STEM_ALL_Z
				 ) ]
		   );
$EXPORT_TAGS{standard} = [ @{ $EXPORT_TAGS{'ops'} },
			   @{ $EXPORT_TAGS{'db'} },
			   @{ $EXPORT_TAGS{'qpflags'} },
			   @{ $EXPORT_TAGS{'qpstem'} } ];
$EXPORT_TAGS{all} = [ @{ $EXPORT_TAGS{'standard'} }, @{ $EXPORT_TAGS{'enq_order'} } ];

our @EXPORT_OK = ( @{ $EXPORT_TAGS{'all'} } );

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

# Compatibility wrapping for Xapian::BAD_VALUENO (wrapped as a constant since
# xapian-bindings 1.4.10).
our $BAD_VALUENO = BAD_VALUENO;

sub search_xapian_compat {
    *Search::Xapian:: = \%Xapian::;
    *Search::Xapian::VERSION = \$VERSION;
    *Search::Xapian::OP_NAMES = \@OP_NAMES;
    *Search::Xapian::DB_NAMES = \@DB_NAMES;
    *Search::Xapian::FLAG_NAMES = \@FLAG_NAMES;
    *Search::Xapian::STEM_NAMES = \@STEM_NAMES;
    *Search::Xapian::BAD_VALUENO = \&BAD_VALUENO;
    *Search::Xapian::DB_OPEN = \&DB_OPEN;
    *Search::Xapian::DB_CREATE = \&DB_CREATE;
    *Search::Xapian::DB_CREATE_OR_OPEN = \&DB_CREATE_OR_OPEN;
    *Search::Xapian::DB_CREATE_OR_OVERWRITE = \&DB_CREATE_OR_OVERWRITE;
    *Search::Xapian::version_string = \&version_string;
    *Search::Xapian::major_version = \&major_version;
    *Search::Xapian::minor_version = \&minor_version;
    *Search::Xapian::revision = \&revision;
    *Search::Xapian::sortable_serialise = \&sortable_serialise;
    *Search::Xapian::sortable_unserialise = \&sortable_unserialise;
}

package Xapian::Database;
sub enquire {
  my $self = shift;
  my $enquire = Xapian::Enquire->new( $self );
  if( @_ ) {
    $enquire->set_query( @_ );
  }
  return $enquire;
}

package Xapian::Enquire;
sub matches {
  my $self = shift;
  return $self->get_mset(@_)->items();
}

package Xapian::ESet;
sub items {
  my $self = shift;
  my @array;
  tie( @array, 'Xapian::ESet', $self );
  return @array;
}

use overload '++' => sub { $_[0]->inc() },
	     '--' => sub { $_[0]->dec() },
	     '='  => sub { $_[0]->clone() },
	     'eq' => sub { $_[0]->equal($_[1]) },
	     'ne' => sub { $_[0]->nequal($_[1]) },
	     '==' => sub { $_[0]->equal($_[1]) },
	     '!=' => sub { $_[0]->nequal($_[1]) },
	     'fallback' => 1;

sub clone() {
  my $self = shift;
  my $class = ref( $self );
  my $copy = new( $self );
  bless $copy, $class;
  return $copy;
}

sub TIEARRAY {
  my $class = shift;
  my $eset = shift;
  return bless $eset, $class;
}

sub FETCHSIZE {
    my $self = shift;
    return $self->size();
}

package Xapian::ESetIterator;
use overload '++' => sub { $_[0]->inc() },
	     '--' => sub { $_[0]->dec() },
	     '='  => sub { $_[0]->clone() },
	     'eq' => sub { $_[0]->equal($_[1]) },
	     'ne' => sub { $_[0]->nequal($_[1]) },
	     '==' => sub { $_[0]->equal($_[1]) },
	     '!=' => sub { $_[0]->nequal($_[1]) },
	     'fallback' => 1;

sub clone() {
  my $self = shift;
  my $class = ref( $self );
  my $copy = new( $self );
  bless $copy, $class;
  return $copy;
}

package Xapian::MSet;
sub items {
  my $self = shift;
  my @array;
  tie( @array, 'Xapian::MSet::Tied', $self );
  return @array;
}

sub TIEARRAY {
  my $class = shift;
  my $mset = shift;
  return bless $mset, $class;
}

sub FETCHSIZE {
    my $self = shift;
    return $self->size();
}

package Xapian::MSetIterator;
use overload '++' => sub { $_[0]->inc() },
	     '--' => sub { $_[0]->dec() },
	     '='  => sub { $_[0]->clone() },
	     'eq' => sub { $_[0]->equal($_[1]) },
	     'ne' => sub { $_[0]->nequal($_[1]) },
	     '==' => sub { $_[0]->equal($_[1]) },
	     '!=' => sub { $_[0]->nequal($_[1]) },
	     'fallback' => 1;

sub clone() {
  my $self = shift;
  my $class = ref( $self );
  bless $self, $class;
  return $self;
}

package Xapian::MSet::Tied;
our @ISA = qw(Xapian::MSet);

package Xapian::PositionIterator;
use overload '++' => sub { $_[0]->inc() },
	     '='  => sub { $_[0]->clone() },
	     'eq' => sub { $_[0]->equal($_[1]) },
	     'ne' => sub { $_[0]->nequal($_[1]) },
	     '==' => sub { $_[0]->equal($_[1]) },
	     '!=' => sub { $_[0]->nequal($_[1]) },
	     'fallback' => 1;

sub clone() {
  my $self = shift;
  my $class = ref( $self );
  my $copy = new( $self );
  bless $copy, $class;
  return $copy;
}

package Xapian::PostingIterator;
use overload '++' => sub { $_[0]->inc() },
	     '='  => sub { $_[0]->clone() },
	     'eq' => sub { $_[0]->equal($_[1]) },
	     'ne' => sub { $_[0]->nequal($_[1]) },
	     '==' => sub { $_[0]->equal($_[1]) },
	     '!=' => sub { $_[0]->nequal($_[1]) },
	     'fallback' => 1;

sub clone() {
  my $self = shift;
  my $class = ref( $self );
  my $copy = new( $self );
  bless $copy, $class;
  return $copy;
}

package Xapian::TermGenerator;
sub set_stopper {
    my ($self, $stopper) = @_;
    $self{_stopper} = $stopper;
    set_stopper1( @_ );
}

package Xapian::TermIterator;
use overload '++' => sub { $_[0]->inc() },
	     '='  => sub { $_[0]->clone() },
	     'eq' => sub { $_[0]->equal($_[1]) },
	     'ne' => sub { $_[0]->nequal($_[1]) },
	     '==' => sub { $_[0]->equal($_[1]) },
	     '!=' => sub { $_[0]->nequal($_[1]) },
	     'fallback' => 1;

sub clone() {
  my $self = shift;
  my $class = ref( $self );
  my $copy = new( $self );
  bless $copy, $class;
  return $copy;
}

package Xapian::ValueIterator;
use overload '++' => sub { $_[0]->inc() },
	     '='  => sub { $_[0]->clone() },
	     'eq' => sub { $_[0]->equal($_[1]) },
	     'ne' => sub { $_[0]->nequal($_[1]) },
	     '==' => sub { $_[0]->equal($_[1]) },
	     '!=' => sub { $_[0]->nequal($_[1]) },
	     'fallback' => 1;

sub clone() {
  my $self = shift;
  my $class = ref( $self );
  my $copy = new( $self );
  bless $copy, $class;
  return $copy;
}

# Adding CLONE_SKIP functions
package Xapian::LogicError;
sub CLONE_SKIP { 1 }
package Xapian::PositionIterator;
sub CLONE_SKIP { 1 }
package Xapian::PostingIterator;
sub CLONE_SKIP { 1 }
package Xapian::TermIterator;
sub CLONE_SKIP { 1 }
package Xapian::ValueIterator;
sub CLONE_SKIP { 1 }
package Xapian::Document;
sub CLONE_SKIP { 1 }
package Xapian::PostingSource;
sub CLONE_SKIP { 1 }
package Xapian::ValuePostingSource;
sub CLONE_SKIP { 1 }
package Xapian::ValueWeightPostingSource;
sub CLONE_SKIP { 1 }
package Xapian::ValueMapPostingSource;
sub CLONE_SKIP { 1 }
package Xapian::FixedWeightPostingSource;
sub CLONE_SKIP { 1 }
package Xapian::MSet;
sub CLONE_SKIP { 1 }
package Xapian::MSetIterator;
sub CLONE_SKIP { 1 }
package Xapian::ESet;
sub CLONE_SKIP { 1 }
package Xapian::ESetIterator;
sub CLONE_SKIP { 1 }
package Xapian::RSet;
sub CLONE_SKIP { 1 }
package Xapian::MatchDecider;
sub CLONE_SKIP { 1 }
package Xapian::Enquire;
sub CLONE_SKIP { 1 }
package Xapian::Weight;
sub CLONE_SKIP { 1 }
package Xapian::BoolWeight;
sub CLONE_SKIP { 1 }
package Xapian::BM25Weight;
sub CLONE_SKIP { 1 }
package Xapian::TradWeight;
sub CLONE_SKIP { 1 }
package Xapian::Database;
sub CLONE_SKIP { 1 }
package Xapian::WritableDatabase;
sub CLONE_SKIP { 1 }
package Xapian::Query;
sub MatchAll { Xapianc::new_Query('') }
sub MatchNothing { Xapianc::new_Query() }
sub CLONE_SKIP { 1 }
package Xapian::Stopper;
sub CLONE_SKIP { 1 }
package Xapian::SimpleStopper;
sub CLONE_SKIP { 1 }
package Xapian::RangeProcessor;
sub CLONE_SKIP { 1 }
package Xapian::DateRangeProcessor;
sub CLONE_SKIP { 1 }
package Xapian::NumberRangeProcessor;
sub CLONE_SKIP { 1 }
package Xapian::FieldProcessor;
sub CLONE_SKIP { 1 }
package Xapian::QueryParser;
sub CLONE_SKIP { 1 }
package Xapian::Stem;
sub CLONE_SKIP { 1 }
package Xapian::TermGenerator;
sub CLONE_SKIP { 1 }
package Xapian::Sorter;
sub CLONE_SKIP { 1 }
package Xapian::MultiValueSorter;
sub CLONE_SKIP { 1 }
package Xapian::ReplicationInfo;
sub CLONE_SKIP { 1 }
package Xapian::DatabaseMaster;
sub CLONE_SKIP { 1 }
package Xapian::DatabaseReplica;
sub CLONE_SKIP { 1 }
package Xapian::ValueSetMatchDecider;
sub CLONE_SKIP { 1 }
package Xapian::SerialisationContext;
sub CLONE_SKIP { 1 }
package Xapian::MSet::Tied;
sub CLONE_SKIP { 1 }

# Pod document of Xapian
=encoding utf8
=head1 NAME

Xapian - Perl frontend to the Xapian C++ search library.

=head1 SYNOPSIS

  use Xapian;

  my $parser = Xapian::QueryParser->new();
  my $query = $parser->parse_query( '[QUERY STRING]' );

  my $db = Xapian::Database->new( '[DATABASE DIR]' );
  my $enq = $db->enquire();

  printf "Running query '%s'\n", $query->get_description();

  $enq->set_query( $query );
  my @matches = $enq->matches(0, 10);

  print scalar(@matches) . " results found\n";

  foreach my $match ( @matches ) {
    my $doc = $match->get_document();
    printf "ID %d %d%% [ %s ]\n", $match->get_docid(), $match->get_percent(), $doc->get_data();
  }

=head1 DESCRIPTION

This module is a pretty-much complete wrapping of the Xapian C++ API.  The
main omissions are features which aren't useful to wrap for Perl, such as
Xapian::UTF8Iterator.

This module is generated using SWIG.  It is intended as a replacement for
the older Search::Xapian module which is easier to keep up to date and
which more completely wraps the C++ API.  It is largely compatible with
Search::Xapian, but see the COMPATIBILITY section below if you have code using
Search::Xapian which you want to get working with this new module.

There are some gaps in the POD documentation for wrapped classes, but you
can read the Xapian C++ API documentation at
L<https://xapian.org/docs/apidoc/html/annotated.html> for details of
these.  Alternatively, take a look at the code in the examples and tests.

If you want to use Xapian and the threads module together, make
sure you're using Perl >= 5.8.7 as then Xapian uses CLONE_SKIP to make sure
that the perl wrapper objects aren't copied to new threads - without this the
underlying C++ objects can get destroyed more than once which leads to
undefined behaviour.

If you encounter problems, or have any comments, suggestions, patches, etc
please email the Xapian-discuss mailing list (details of which can be found at
L<https://xapian.org/lists>).

=head2 COMPATIBILITY

This module is mostly compatible with Search::Xapian.  The following are known
differences, with details of how to write code which works with both.

Search::Xapian overloads stringification - e.g. C<"$query"> is equivalent to
C<$query-E<gt>get_description()>, while C<"$termiterator"> is equivalent to
C<$termiterator-E<gt>get_term()>.  This module doesn't support overloaded
stringification, so you should instead explicitly call the method you
want.  The technical reason for this change is that stringification is hard to
support in SWIG-generated bindings, but this context-sensitive stringification
where the operation performed depends on the object type seems unhelpful in
hindsight anyway.

Search::Xapian overloads conversion to an integer for some classes - e.g.
C<0+$positioniterator> is equivalent to C<$positioniterator-E<gt>get_termpos>
while C<0+$postingiterator> is equivalent to C<$postingiterator-E<gt>get_docid>.
This module doesn't provide these overloads so you should instead explicitly
call the method you want.  As above, we think this context-sensitive behaviour
wasn't helpful in hindsight.

This module is fussier about whether a passed scalar value is a string or
an integer than Search::Xapian, so e.g. C<Xapian::Query-E<gt>new(2001)> will fail
but the equivalent worked with Search::Xapian.  If C<$term> might not be a
string use C<Xapian::Query-E<gt>new("$term")> to ensure it is converted to a
string.  The new behaviour isn't very Perlish, but is likely to be hard to
address universally as it comes from SWIG.  Let us know if you find particular
places where it's annoying and we can look at addressing those.

Both this module and Search::Xapian support passing a Perl sub (which can be
anonymous) for the functor classes C<MatchDecider> and C<ExpandDecider>.  In
some cases Search::Xapian accepts a string naming a Perl sub, but this module
never accepts this.  Instead of passing C<"::mymatchdecider">, pass
C<\&mymatchdecider> which will work with either module.  If you really want to
dynamically specify the function name, you can pass C<sub {eval
"&$dynamicmatchdecider"}>.

Search::Xapian provides a PerlStopper class which is supposed to be
subclassable in Perl to implement your own stopper, but this mechanism doesn't
actually seem to work.  This module instead supports user-implemented stoppers
by accepting a Perl sub in place of a Stopper object.

=head3 Importing Either Module

If you want your code to use either this module or Search::Xapian depending
what's installed, then instead of C<use Search::Xapian (':all');> you can use:

  BEGIN {
    eval {
      require Xapian;
      Xapian->import(':all');
      Xapian::search_xapian_compat();
    };
    if ($@) {
      require Search::Xapian;
      Search::Xapian->import(':all');
    }
  }

If you just C<use Search::Xapian;> then the C<import()> calls aren't needed.

The C<Xapian::search_xapian_compat()> call sets up aliases in the
C<Search::Xapian> namespace so you can write code which refers to
C<Search::Xapian> but can actually use this module instead.

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
a parameter to C<Xapian::Query-E<gt>new()>, but it defaults to the
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

Support AND, OR, etc and bracketted subexpressions.

=item FLAG_LOVEHATE

Support + and -.

=item FLAG_PHRASE

Support quoted phrases.

=item FLAG_BOOLEAN_ANY_CASE

Support AND, OR, etc even if they aren't in ALLCAPS.

=item FLAG_WILDCARD

Support right truncation (e.g. Xap*).

=item FLAG_WILDCARD_GLOB

=item FLAG_WILDCARD_MULTI

=item FLAG_WILDCARD_SINGLE

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

=item FLAG_ACCUMULATE

=item FLAG_AUTO_SYNONYMS

=item FLAG_AUTO_MULTIWORD_SYNONYMS

=item FLAG_CJK_NGRAM

=item FLAG_CJK_WORDS

=item FLAG_FUZZY

=item FLAG_NO_POSITIONS

=back

=head1 :qpstem

=over 4

=item STEM_ALL

Stem all terms.

=item STEM_ALL_Z

Stem all terms and add a "Z" prefix.

=item STEM_NONE

Don't stem any terms.

=item STEM_SOME

Stem some terms, in a manner compatible with Omega (capitalised words and those
in phrases aren't stemmed).

=item STEM_SOME_FULL_POS

Like STEM_SOME but also store term positions for stemmed terms.

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
Xapian 1.4.15 this would return 1.

=item minor_version

Returns the minor version of the Xapian C++ library being used.  E.g. for
Xapian 1.4.15 this would return 4.

=item revision

Returns the revision of the Xapian C++ library being used.  E.g. for
Xapian 1.4.15 this would return 15.  In a stable release series, Xapian
libraries with the same minor and major versions are usually ABI compatible, so
this often won't match the third component of C<$Xapian::VERSION> (which is the
version of the Xapian wrappers).

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

This expects the input to be a string produced by C<sortable_serialise()>.
If the input is not such a string, the value returned is undefined (but
no error will be thrown).

The result of the conversion will be exactly the value which was
supplied to C<sortable_serialise()> when making the string on platforms
which represent doubles with the precisions specified by IEEE_754, but
may be a different (nearby) value on other platforms.

=back

=head1 TODO

=over 4

=item Documentation

Add POD documentation for all classes, where possible just adapted from Xapian
docs.

=item Unwrapped classes

The following Xapian classes are not yet wrapped:
user-defined Weight subclasses.

=back

=head1 CREDITS

These SWIG-generated Perl bindings were originally implemented by Kosei
Moriyama in GSoC 2009, and made their debut in the 1.2.4 release.

They take a lot of inspiration and some code from Search::Xapian, a set
of hand-written XS bindings, originally written by Alex Bowley, and later
maintained by Olly Betts.

Search::Xapian owed thanks to Tye McQueen E<lt>tye@metronet.comE<gt> for
explaining the finer points of how best to write XS frontends to C++ libraries,
and James Aylett E<lt>james@tartarus.orgE<gt> for clarifying the less obvious
aspects of the Xapian API.  Patches for wrapping missing classes and other
things were contributed by Olly Betts, Tim Brody, Marcus Ramberg, Peter Karman,
Benjamin Smith, Rusty Conover, Frank Lichtenheld, Henry Combrinck, Jess
Robinson, David F. Skoll, Dave O'Neill, Andreas Marienborg, Adam Sjøgren,
Dmitry Karasik, and Val Rosca.

=head1 AUTHOR

Please report any bugs/suggestions to E<lt>xapian-discuss@lists.xapian.orgE<gt>
or use the Xapian bug tracker L<https://xapian.org/bugs>.  Please do
NOT use the CPAN bug tracker or mail contributors individually.

=head1 LICENSE

This program is free software; you can redistribute it and/or modify
it under the same terms as Perl itself.

=head1 SEE ALSO

L<Xapian::BM25Weight>,
L<Xapian::BoolWeight>,
L<Xapian::Database>,
L<Xapian::Document>,
L<Xapian::Enquire>,
L<Xapian::MultiValueSorter>,
L<Xapian::PositionIterator>,
L<Xapian::PostingIterator>,
L<Xapian::Query>,
L<Xapian::QueryParser>,
L<Xapian::Stem>,
L<Xapian::TermGenerator>,
L<Xapian::TermIterator>,
L<Xapian::TradWeight>,
L<Xapian::ValueIterator>,
L<Xapian::Weight>,
L<Xapian::WritableDatabase>,
and
L<https://xapian.org/>.

=cut
%}
