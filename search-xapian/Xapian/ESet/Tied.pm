package Search::Xapian::ESet::Tied;

use 5.006;
use strict;
use warnings;

use Tie::Array;
use Search::Xapian::ESet;

require DynaLoader;

our @ISA = qw(Search::Xapian::ESet Tie::Array DynaLoader);

# Preloaded methods go here.

sub TIEARRAY {
  my $class = shift;
  my $eset = shift;
  return bless $eset, $class;
}

sub FETCH {
  my ($self, $index) = @_;
  return $self->get_esetiterator( $index );
}

sub FETCHSIZE {
  my ($self) = @_;
  return $self->size();
}

1;
