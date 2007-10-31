package Search::Xapian::MSet::Tied;

use 5.006;
use strict;
use warnings;

use Tie::Array;
use Search::Xapian::MSet;

require DynaLoader;

our @ISA = qw(Search::Xapian::MSet Tie::Array DynaLoader);

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
