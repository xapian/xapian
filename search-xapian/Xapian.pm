package Search::Xapian;

use 5.006;
use strict;
use warnings;
use Carp;

use Search::Xapian::Database;
use Search::Xapian::Document;
use Search::Xapian::Enquire;
use Search::Xapian::MSet;
use Search::Xapian::MSetIterator;
use Search::Xapian::Query;
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
                                  OP_WEIGHT_CUTOFF
                                  OP_ELITE_SET
                                 ) ],
                    'db' => [ qw(
                                 OM_DB_OPEN
                                 OM_DB_CREATE
                                 ) ]
                   );
$EXPORT_TAGS{standard} = [ @{ $EXPORT_TAGS{'ops'} }, @{ $EXPORT_TAGS{'db'} } ];
$EXPORT_TAGS{all} = [ @{ $EXPORT_TAGS{'standard'} } ];


our @EXPORT_OK = ( @{ $EXPORT_TAGS{'all'} } );

our @EXPORT = qw( );


our $VERSION = '0.04';

bootstrap Search::Xapian $VERSION;

# Preloaded methods go here.

1;

__END__


=head1 NAME

Search::Xapian - Perl XS frontend to the Xapian C++ search library.

=head1 SYNOPSIS

  use Search::Xapian;

  my $db = Search::Xapian::Database->new( '[DATABASE DIR]' );
  my $enq = $db->enquire( '[QUERY TERM]' );

  printf "Parsing query '%s'\n", $enq->get_query()->get_description();

  my @matches = $enq->matches(0, 10);

  print scalar(@matches) . " results found\n";

  foreach my $match ( @matches ) {
    my $doc = $match->get_document();
    printf "ID %d %d%% [ %s ]\n", $match->get_docid(), $match->get_percent(), $doc->get_data();
  }

=head1 DESCRIPTION

This module provides access to most of the classes in the xapian
library, as well as a more simplified, 'perlish' interface - as
demonstrated above.

The xapian library is evolving very quickly at the time of writing,
hence any documentation placed here would be likely to become out of
date quite rapidly, and I do not have the patience to write some which
could rapidly become redundant.

Apologies to those of you considering using this module. For the time
being, I would suggest garnering what you can from the tests and
examples provided with this module, or reading through the xapian
documentation on http://www.xapian.org/.

If you encounter problems, email either me or preferably the
xapian-discuss mailing list (which I am on - subscription details can
be found on the xapian web site).

=head2 EXPORT

None by default.

=head1 TODO

=over 4

=item Error Handling

Error handling for all method liable to generate them.

=item Documentation

Brief descriptions of classes, possibly just adapted for xapian docs.

=head1 AUTHOR

Alex Bowley E<lt>kilinrax@cpan.orgE<gt>

=head1 SEE ALSO

L<perl>. L<xapian>.

=cut
