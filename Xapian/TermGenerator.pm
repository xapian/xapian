package Search::Xapian::TermGenerator;

use 5.006;
use strict;
use warnings;
use Carp;

require Exporter;
require DynaLoader;

our @ISA = qw(Exporter DynaLoader);
# Items to export into caller's namespace by default. Note: do not export
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
  my $tg = new0();
  
  bless $tg, $class;

  return $tg;
}

1;

__END__

=head1 NAME

Search::Xapian::TermGenerator - Parses a piece of text and generates terms.

=head1 DESCRIPTION

This module takes a piece of text and parses it to produce words which are
then used to generate suitable terms for indexing.  The terms generated
are suitable for use with L<Search::Xapian::Query> objects produced by the
L<Search::Xapian::QueryParser> class.

=head1 SYNOPSIS

  use Search::Xapian;

  my $doc = new Search::Xapian::Document();
  my $tg = new Search::Xapian::TermGenerator();
  $tg->set_stemmer(new Search::Xapian::Stem("english"));
  $tg->set_document($doc);
  $tg->index_text("The cat sat on the mat");

=head1 METHODS

=over 4

=item new

TermGenerator constructor.

=item set_stemmer <stemmer>

Set the L<Search::Xapian::Stem> object to be used for generating stemmed terms.

=item set_stopper <stopper>

Set the L<Search::Xapian::Stopper> object to be used for identifying stopwords.

=item set_document <document>

Set the L<Search::Xapian::Document> object to index terms into.

=item get_document <document>

Get the currently set L<Search::Xapian::Document> object.

=item index_text <text> [<weight> [<prefix>]]

Indexes the text in string <text>.  The optional parameter <weight> sets the
wdf increment (default 1).  The optional parameter <prefix> sets the term
prefix to use (default is no prefix).

=item index_text_without_positions <text> [<weight> [<prefix>]]

Just like index_text, but no positional information is generated.  This means
that the database will be significantly smaller, but that phrase searching
and NEAR won't be supported.

=item increase_termpos [<delta>]

Increase the termpos used by index_text by <delta> (default 100).

This can be used to prevent phrase searches from spanning two
unconnected blocks of text (e.g. the title and body text).

=item get_termpos

Get the current term position.

=item set_termpos <termpos>

Set the current term position.

=item get_description

Returns a string describing this object.  (for introspection)

=back

=head1 REFERENCE

  http://www.xapian.org/docs/sourcedoc/html/classXapian_1_1TermGenerator.html

=cut
