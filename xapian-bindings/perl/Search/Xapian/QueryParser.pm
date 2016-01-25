package Search::Xapian::QueryParser;

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

Set default operator for joining elements. Useful values are
OP_AND and OP_OR.  See L<Search::Xapian> for descriptions of these constants.

=item get_default_op

Returns the default operator for joining elements.

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

=back

=head1 REFERENCE

  https://xapian.org/docs/queryparser.html
  https://xapian.org/docs/sourcedoc/html/classXapian_1_1QueryParser.html

=cut
1;
