package Search::Xapian::MSet;

use 5.006;
use strict;
use warnings;
use Carp;

use Search::Xapian::MSet::Tied;

use Tie::Array;
require DynaLoader;

our @ISA = qw(Tie::Array DynaLoader);

# Preloaded methods go here.

# In a new thread, copy objects of this class to unblessed, undef values.
sub CLONE_SKIP { 1 }

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
  my $mset;
  my $invalid_args;
  if( scalar(@_) == 0 ) {
    $mset = new1();
  } elsif( scalar(@_) == 1 and ref( $_[1] ) eq $class ) {
    $mset = new2(@_);
  } else {
    $invalid_args = 1;
  }
  if( $invalid_args ) {
    Carp::carp( "USAGE: $class->new(), $class->new(\$mset)" );
    exit;
  }
  bless $mset, $class;
  return $mset;
}

sub fetch() {
  my $self = shift;
  my $invalid_args;
  if( scalar(@_) == 2 ) {
    $self->fetch1(@_);
  } elsif( scalar(@_) == 1 ) {
    $self->fetch2(@_);
  } elsif( scalar(@_) == 0 ) {
    $self->fetch3();
  } else {
    $invalid_args = 1;
  }
  if( $invalid_args ) {
    Carp::carp( "USAGE: \$mset->fetch(\$begin, \$end), \$mset->fetch(\$msetiterator), \$mset->fetch()" );
    exit;
  }
}

sub convert_to_percent() {
  my $self = shift;
  my $invalid_args;
  if( scalar(@_) == 1 ) {
    my $arg = shift;
    my $arg_class = ref $arg;
    if( !$arg_class ) {
      return $self->convert_to_percent1($arg);
    } elsif( $arg_class eq 'Search::Xapian::MSetIterator' ) {
      return $self->convert_to_percent2($arg);
    } else {
      $invalid_args = 1;
    }
  } else {
    $invalid_args = 1;
  }
  if( $invalid_args ) {
    Carp::carp( "USAGE: \$enquire->convert_to_percent(\$weight) or \$enquire->convert_to_percent(\$msetiterator)" );
    exit;
  }
}

sub items {
  my $self = shift;
  my @array;
  tie( @array, 'Search::Xapian::MSet::Tied', $self );
  return @array;
}

sub TIEARRAY {
  my $class = shift;
  my $eset = shift;
  return bless $eset, $class;
}

1;
