package Search::Xapian::TermGenerator;

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

Return a description of this object.

=back

=head1 REFERENCE

  https://xapian.org/docs/sourcedoc/html/classXapian_1_1TermGenerator.html

=cut
1;
