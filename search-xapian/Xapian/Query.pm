package Search::Xapian::Query;

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
  my $query;
  my $invalid_args;
  if( scalar(@_) == 1 ) {
    $query = new1(@_);
  } elsif( scalar(@_) == 3 ) {
    if( !ref( $_[1] ) and !ref( $_[2] ) ) {
      $query = new2(@_);
    } elsif( ref( $_[1] ) eq ref( $_[2] ) and ref( $_[2] ) eq $class ) {
      $query = new3(@_);
    } else {
        Carp::carp( "new()'s second and third arguments must both either be search terms (scalars), or $class objects" );
        $invalid_args++;
    }
  } else {
    my $op = shift;
    foreach( @_ ) {
      if( ref ) {
        Carp::carp( "When new() is called with variable arguments, all after the first must be search terms (scalars) ");
        $invalid_args++;
        last;
      }
    }
    while( @_ ) {
      my $temp;
      if( !$#_ ) {
        $temp = new1( shift );
      } else {
        $temp = new2( $op, shift, shift );
      }
      bless $temp, $class;
      $query = defined($query) ? new3( $op, $query, $temp ) : $temp;
      bless $query, $class;
    }
    return $query;
    exit;
  }
  if( $invalid_args ) {
    Carp::carp( "USAGE: $class->new('term'), $class->new(OP, 'term1', 'term2'), $class->new(OP, \$query1, \$query2) or $class->new(OP, \@terms)" );
    exit;
  }
  bless $query, $class;
  return $query;
}

1;
