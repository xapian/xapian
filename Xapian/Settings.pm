package Search::Xapian::Settings;

use 5.006;
use strict;
use warnings;
use Carp;

require Exporter;
require DynaLoader;

our @ISA = qw(Exporter DynaLoader);
# Items to export into callers namespace by default. Note: do not export
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
  my $settings;
  my $invalid_args;
  if( scalar(@_) == 0 ) {
    $settings = new1();
  } elsif( scalar(@_) == 1 and ref( $_[1] ) eq $class ) {
    $settings = new2(@_);
  } else {
    $invalid_args = 1;
  }
  if( $invalid_args ) {
    Carp::carp( "USAGE: $class->new(), $class->new(\$settings)" );
    exit;
  }
  bless $settings, $class;
  return $settings;
}

sub set() {
  my $self = shift;
  my $invalid_args;
  if( scalar(@_) == 2 ) {
    my ($key, $value) = @_;
    if( $value =~ /^[+-]?\d+$/ ) {
      # integer
      $self->set2($key, $value);
    } elsif( $value =~ /^[+-]?(?=\d|\.\d)\d*(?:\.\d*)?(?:[Ee][+-]?\d+)?$/ ) {
      # real
      $self->set3($key, $value);
    } else {
      # string
      $self->set1($key, $value);
    }
  } else {
    $invalid_args = 1;
  }
  if( $invalid_args ) {
    Carp::carp( "USAGE: \$settings->set(\$key, \$value)" );
    exit;
  }
}

1;
