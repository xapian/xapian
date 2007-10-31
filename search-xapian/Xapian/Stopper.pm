package Search::Xapian::Stopper;

use 5.006;
use strict;
use warnings;
use Carp;

require DynaLoader;

our @ISA = qw(DynaLoader);

# Preloaded methods go here.

# In a new thread, copy objects of this class to unblessed, undef values.
sub CLONE_SKIP { 1 }

sub new() {
  my $class = shift;
  my $stopper;
  my $invalid_args;
  if( scalar(@_) == 0 ) {
    $stopper = new1();
  } else {
    $stopper = new2(@_);
  }
  if( $invalid_args ) {
    Carp::carp( "USAGE: $class->new(), $class->new(\@words)" );
    exit;
  }
  bless $stopper, $class;
  return $stopper;
}

1;
