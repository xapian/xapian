package Search::Xapian::MSet::Tied;

use 5.006;
use strict;
use warnings;
use Carp;

use Tie::Array;
use Search::Xapian::MSet;

require Exporter;
require DynaLoader;

our @ISA = qw(Search::Xapian::MSet Tie::Array Exporter DynaLoader);
# Items to export into callers namespace by default. Note: do not export
# names by default without a very good reason. Use EXPORT_OK instead.
# Do not simply export all your public functions/methods/constants.

# If you do not need this, moving things directly into @EXPORT or @EXPORT_OK
# will save memory.
our @EXPORT_OK = ( );

our @EXPORT = qw( );


# Preloaded methods go here.

sub TIEARRAY {
  my $class = shift;
  my $mset = shift;
  return bless $mset, $class;
}

sub FETCH {
  my ($self, $index) = @_;
  return $self->get_msetiterator( $index );
}

sub FETCHSIZE {
  my ($self) = @_;
  return $self->size();
}

1;
