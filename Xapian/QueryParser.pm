package Search::Xapian::QueryParser;

use 5.006;
use strict;
use warnings;
use Carp;

require Exporter;
require DynaLoader;

our @ISA = qw(Exporter DynaLoader);
# Items to export into callers namespace by default. Note: do not export
# names by default without a very good reason. Use EXPORT_OK instead.
# Do not simply export all your public functions/methods/constants.

# If you do not need this, moving things directly into @EXPORT or @EXPORT_OK
# will save memory.
our @EXPORT_OK = ( );

our @EXPORT = qw( );

# Preloaded methods go here.

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

Search::Xapian::QueryParser - Parse a query string into a Query object

=head1 DESCRIPTION

This module turns a human readable string into something Xapian can
understand.  The syntax supported is designed to be similar to other web
based search engines, so that users familiar with them don't have to learn
a whole new syntax.

=head1 SYNOPSIS

  use Search::Xapian qw/:standard/;

  my $qp = new Search::Xapian::QueryParser( [$database] );
  $qp->set_stemming_options("english",1); # Replace 1 with 0 if you want to disable stemming
  $qp->set_default_op(OP_AND);

  $database->enquire($qp->parse_query('a word OR two NEAR "a phrase" NOT (too difficult) +eh'));
 
=head1 METHODS

=over 4

=item new <database>

QueryParser constructor.

=item set_stemming_options <language> <enabled>

Set language for stemming, and weither it's going to be used at all
enable can be 1 or 0.

Currently supported options for language: danish, dutch, english, finnish, 
french, german, italian, norwegian, portuguese, russian, spanish, and swedish
 
=item set_default_op <operator>

Set default operator for joining elements. Can be one of  OP_AND, OP_OR,
OP_AND_NOT, OP_XOR, OP_AND_MAYBE, OP_FILTER,OP_NEAR,OP_PHRASE or OP_ELITE_SET.
See L<Search::Xapian> for descriptions of these constants.

=item get_default_op

Returns the default operator for joining elements.

=item set_database <database>

Pass a L<Search::Xapian::Database> object to be used for searching.

=item parse_query <query_string> [<flags>]

parses the query string according to the rules defined in the query parser
documentation below. Allows you to specify certain flags to modify the
searching behavior:

  FLAG_BOOLEAN=1, FLAG_PHRASE=2, FLAG_LOVEHATE=4, 
  FLAG_BOOLEAN_ANY_CASE=8, FLAG_WILDCARD = 16 

default flags are FLAG_PHRASE,FLAG_BOOLEAN and FLAG_LOVEHATE

=item add_prefix <field> <prefix>

Add a probabilistic term prefix.  E.g. $qp->add_prefix("author", "A");

Allows the user to search for author:orwell which will search for the term "Aorwel" (assuming English stemming is in use). Multiple fields can be mapped to the same prefix (so you can e.g. make title: and subject: aliases for each other).

Parameters:
field 	The user visible field name
prefix 	The term prefix to map this to 

=item add_boolean_prefix <field> prefix

Add a boolean term prefix allowing the user to restrict a search with a 
boolean filter specified in the free text query.  E.g. 
$p->add_boolean_prefix("site", "H");

Allows the user to restrict a search with site:xapian.org which will be 
converted to Hxapian.org combined with any probabilistic query with OP_FILTER.

Multiple fields can be mapped to the same prefix (so you can e.g. make site: 
and domain: aliases for each other).

Parameters:
field 	The user visible field name
prefix 	The term prefix to map this t 

=item 

=back
 
=head1 REFERENCE

  http://www.xapian.org/docs/queryparser.html
  http://www.xapian.org/docs/sourcedoc/html/classXapian_1_1QueryParser.html

=head1 TODO

Implement Xapian::Stopper.

=cut
