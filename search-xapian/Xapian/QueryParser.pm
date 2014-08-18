package Search::Xapian::QueryParser;

use 5.006;
use strict;
use warnings;

require DynaLoader;

our @ISA = qw(DynaLoader);

# Preloaded methods go here.

# In a new thread, copy objects of this class to unblessed, undef values.
sub CLONE_SKIP { 1 }

use overload '='  => sub { $_[0]->clone() },
	     'fallback' => 1;

sub clone() {
  my $self = shift;
  my $class = ref( $self );
  my $copy = new2( $self );
  bless $copy, $class;
  return $copy;
}

sub new() {
  my $class = shift;
  my $qp = new0();

  bless $qp, $class;
  $qp->set_database(@_) if scalar(@_) == 1;

  return $qp;
}

1;

__END__

=head1 NAME

Search::Xapian::QueryParser - Parse a query string into a Search::Xapian::Query object

=head1 DESCRIPTION

This module turns a human readable string into something Xapian can
understand.  The syntax supported is designed to be similar to other web
based search engines, so that users familiar with them don't have to learn
a whole new syntax.

=head1 SYNOPSIS

  use Search::Xapian qw/:standard/;

  my $qp = new Search::Xapian::QueryParser( [$database] );
  $qp->set_stemmer(new Search::Xapian::Stem("english"));
  $qp->set_default_op(OP_AND);

  $database->enquire($qp->parse_query('a NEAR word OR "a phrase" NOT (too difficult) +eh'));

=head1 METHODS

=over 4

=item new <database>

QueryParser constructor.

=item set_stemmer <stemmer>

Set the Search::Xapian::Stem object to be used for stemming query terms.

=item set_stemming_strategy <strategy>

Set the stemming strategy.  Valid values are C<STEM_ALL>, C<STEM_SOME>,
C<STEM_NONE>.

=item set_stopper <stopper>

Set the Search::Xapian::Stopper object to be used for identifying stopwords.

=item set_default_op <operator>

Set the default operator.

This operator is used to combine non-filter query items when no
explicit operator is used.

The most useful values for this are OP_OR (the default) and OP_AND.
OP_NEAR and OP_PHRASE can also be useful.

See L<Search::Xapian> for descriptions of these constants.

=item get_default_op

Returns the current default operator.

=item set_database <database>

Pass a L<Search::Xapian::Database> object which is used to check whether
terms exist in some situations.

=item parse_query <query_string> [<flags>]

Parses the query string according to the rules defined in the query parser
documentation below. You can specify certain flags to modify the
searching behaviour:

  FLAG_BOOLEAN, FLAG_PHRASE, FLAG_LOVEHATE, FLAG_BOOLEAN_ANY_CASE,
  FLAG_WILDCARD, FLAG_PURE_NOT, FLAG_PARTIAL, FLAG_SPELLING_CORRECTION,
  FLAG_SYNONYM, FLAG_AUTO_SYNONYMS, FLAG_AUTO_MULTIWORD_SYNONYMS

To specify multiple flags, "bitwise or" them together (with C<|>).  The
default flags are C<FLAG_PHRASE|FLAG_BOOLEAN|FLAG_LOVEHATE>

=item add_prefix <field> <prefix>

Add a probabilistic term prefix.  E.g. $qp->add_prefix("author", "A");

Allows the user to search for author:orwell which will search for the term
"Aorwel" (assuming English stemming is in use). Multiple fields can be mapped
to the same prefix (so you can e.g. make title: and subject: aliases for each
other).

Parameters:
field	The user visible field name
prefix	The term prefix to map this to

=item add_boolean_prefix <field> prefix

Add a boolean term prefix allowing the user to restrict a search with a
boolean filter specified in the free text query.  E.g.

  $p->add_boolean_prefix("site", "H");

Allows the user to restrict a search with site:xapian.org which will be
converted to Hxapian.org combined with any probabilistic query with
C<OP_FILTER>.

Multiple fields can be mapped to the same prefix (so you can e.g. make site:
and domain: aliases for each other).

Parameters:
field	The user visible field name
prefix	The term prefix to map this to

=item stoplist_begin

=item stoplist_end

=item unstem_begin

=item unstem_end

=item get_description

Returns a string describing this object.

=item get_corrected_query_string

Get the spelling-corrected query string.

This will only be set if FLAG_SPELLING_CORRECTION is specified when
QueryParser::parse_query() was last called.

If there were no corrections, an empty string is returned.

=item set_max_wildcard_expansion <limit>

Specify the maximum expansion of a wildcard term.

Note: you must also set FLAG_WILDCARD for wildcard expansion to happen.

Parameter limit is the maximum number of terms each wildcard in the query can
expand to, or 0 for no limit (which is the default).

=cut

=back

=head1 REFERENCE

  http://www.xapian.org/docs/queryparser.html
  http://www.xapian.org/docs/sourcedoc/html/classXapian_1_1QueryParser.html

=cut
