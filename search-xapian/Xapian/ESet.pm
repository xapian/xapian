package Search::Xapian::ESet;

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
    Carp::carp( "USAGE: $class->new(), $class->new(\$eset)" );
    exit;
  }
  bless $mset, $class;
  return $mset;
}

1;
