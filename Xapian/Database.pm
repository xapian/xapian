package Search::Xapian::Database;

use 5.006;
use strict;
use warnings;
use Carp;

use Search::Xapian::Enquire;

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

sub enquire {
  my $self = shift;
  my $enquire = Search::Xapian::Enquire->new( $self );
  if( @_ ) {
    $enquire->set_query( @_ );
  }
  return $enquire;
}

sub clone() {
  my $self = shift;
  my $class = ref( $self );
  my $copy = new2( $self );
  bless $copy, $class;
  return $copy;
}

sub new() {
  my $class = shift;
  my $database;
  my $invalid_args;
  if( scalar(@_) == 1 ) {
    my $arg = shift;
    my $arg_class = ref( $arg );
    if( !$arg_class ) {
      $database = new1( $arg );
    } elsif( $arg_class eq $class ) {
      $database = new2( $arg );
    } else {
      $invalid_args = 1;
    }
  } else {
    $invalid_args = 1;
  }
  if( $invalid_args ) {
    Carp::carp( "USAGE: $class->new(\$file), $class->new(\$database)" );
    exit;
  }
  bless $database, $class;
  return $database;
}

1;
