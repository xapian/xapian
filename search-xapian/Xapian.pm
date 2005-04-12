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
use Search::Xapian::Stopper;
use Search::Xapian::TermIterator;
use Search::Xapian::ValueIterator;
use Search::Xapian::WritableDatabase;

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
                                  OP_ELITE_SET
                                 ) ],
                    'db' => [ qw(
                                 DB_OPEN
                                 DB_CREATE
                                 DB_CREATE_OR_OPEN
                                 DB_CREATE_OR_OVERWRITE
                                 ) ]
                   );
$EXPORT_TAGS{standard} = [ @{ $EXPORT_TAGS{'ops'} }, @{ $EXPORT_TAGS{'db'} } ];
$EXPORT_TAGS{all} = [ @{ $EXPORT_TAGS{'standard'} } ];


our @EXPORT_OK = ( @{ $EXPORT_TAGS{'all'} } );

our @EXPORT = qw( );


our $VERSION = '0.8.4.0';

bootstrap Search::Xapian $VERSION;

# Preloaded methods go here.

our @OP_NAMES;
foreach (@{ $EXPORT_TAGS{'ops'} }) {
  $OP_NAMES[eval $_] = $_;
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
http://www.xapian.org/, notably the API documentation at
http://www.xapian.org/docs/apidoc/html/annotated.html

If you encounter problems, email either me or preferably the
Xapian-discuss mailing list (which I am on - subscription details can
be found on the Xapian web site).

=head2 EXPORT

None by default.

=head1 TODO

=over 4

=item Error Handling

Error handling for all methods liable to generate them.

=item Documentation

Brief descriptions of classes, possibly just adapted from Xapian docs.

=item Unwrapped classes

The following Xapian classes are not yet wrapped:
Error (and subclasses), ErrorHandler, ExpandDecider (and subclasses),
MatchDecider, Weight (and subclasses).

=item Unwrapped methods

The following methods are not yet wrapped:
Enquire::get_eset(...) with more than two arguments,
Enquire::get_mset(...) with more than two arguments,
Enquire::register_match_decoder(...) with one argument,
Enquire::set_weighting_scheme(const Weight &weight);
Query::Query(tname, ...); with more than one argument;
QueryParser::set_stemming_options() with third (Stopper) argument;

=head1 CREDITS

Thanks to Tye McQueen E<lt>tye@metronet.comE<gt> for explaining the
finer points of how best to write XS frontends to C++ libraries, James
Aylett E<lt>james@tartarus.orgE<gt> for clarifying the less obvious
aspects of the Xapian API, and especially Olly Betts
E<lt>olly@survex.comE<gt> for contributing advice, bugfixes, and
wrapper code for the more obscure classes.

=head1 AUTHOR

Alex Bowley E<lt>kilinrax@cpan.orgE<gt>

=head1 SEE ALSO

L<perl>. L<xapian>.

=cut
