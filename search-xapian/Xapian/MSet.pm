package Search::Xapian::MSet;

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

1;
