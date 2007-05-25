package Search::Xapian::Stopper;

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
