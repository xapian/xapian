package Search::Xapian;

use 5.006;
use strict;
use warnings;

require Exporter;
require DynaLoader;

our @ISA = qw(Exporter DynaLoader);

# Items to export into callers namespace by default. Note: do not export
# names by default without a very good reason. Use EXPORT_OK instead.
# Do not simply export all your public functions/methods/constants.

# This allows declaration	use Search::Xapian ':all';
# If you do not need this, moving things directly into @EXPORT or @EXPORT_OK
# will save memory.
our %EXPORT_TAGS = ( 'all' => [ qw(
	
) ] );

our @EXPORT_OK = ( @{ $EXPORT_TAGS{'all'} } );

our @EXPORT = qw(
	
);
our $VERSION = '0.01';

bootstrap Search::Xapian $VERSION;

# Preloaded methods go here.

package Search::Xapian::MatchSet;

sub matches {
  my $mset = shift;
  my $i = $mset->begin();
  my $size = $mset->size();
  my $tmp = $i;
  my @results = $tmp;
  print $i->get_percent() . "\n";
    print $results[0]->get_percent() . "\n";
  while(--$size) {
    print $results[0]->get_percent() . "\n";
    $tmp = $i;
    $i = $tmp->inc();
    push @results, $i;
  }
    print $results[0]->get_percent() . "\n";
  return @results;
}

1;
__END__

=head1 NAME

Search::Xapian - Perl XS frontend to the Xapian C++ search library.

=head1 SYNOPSIS

  use Search::Xapian;

  my $settings = Search::Xapian::Settings->new();

  $settings->set( 'backend', 'auto' );
  $settings->set( 'auto_dir', '[DATABASE DIR]' );

  my $db = Search::Xapian::Database->new( $settings );
  my $enq = Search::Xapian::Enquire->new( $db );
  my $query = Search::Xapian::Query->new( '[QUERY TERM]' );

  printf "Parsing query '%s'\n", $query->get_description();

  $enq->set_query( $query );

  my $matches = $enq->get_mset( 0, 10 );

  printf "%d results found\n", $matches->get_estimated();

  my $match = $matches->begin();
  my $size = $matches->size();

  while( $size-- ){
    printf "ID %d %d%% [ %s ]\n", $match->get_docid(), $match->get_percent(), $match->get_document()->get_data();
    $match->inc();
  }

=head1 DESCRIPTION

Currently this module only provides objects required for searching,
not indexing. Expect this to change in the near future.

More detailed documentation on xapian can be found at http://www.xapian.org/

=head2 EXPORT

None by default.

=head1 TODO

=over 4

=item Indexing

Bindings for OmWriteableDatabase, OmStem, and other classes needed for
indexing and database creation.

=item Tests

Separate out test scripts and examples.

=item Error Handling

Returning OmErrors to Perl in a sane manner.

=item Documentation

Brief descriptions of classes, possibly just adapted for xapian docs.

=item Search::Xapian::Simple

To provide a simplified, more 'Perlish' interface.

=head1 AUTHOR

Alex Bowley E<lt>kilinrax@cpan.orgE<gt>

=head1 SEE ALSO

L<perl>. L<xapian>.

=cut
