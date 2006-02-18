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

use overload '""' => sub { $_[0]->get_description() }, # FIXME: perhaps unwise?
             'fallback' => 1;

sub new {
  my $class = shift;
  my $query;
  my $invalid_args;

  if( scalar(@_) == 1 ) {
    $query = new1(@_);
  }
  else {
    my $op = shift;
    if( $op !~ /^[-+]?\d+$/ ) {
      Carp::carp( "new()'s first argument must be an OP when called with more than one argument" );
      $invalid_args++;
    }
    else {
      if( !_all_equal( map { ref } @_ ) ) {
        Carp::carp( "all of new()'s arguments after the first must be of identical type (either all search terms (scalars) or $class objects)");
        $invalid_args++;
      }
      else {
        # remaining arguments are scalars
        if( !ref($_[0]) ) {
          scalar(@_) == 1 ?
            $query = new2sv($op, @_) :
              $query = newXsv($op, @_);
        }
        # remaining arguments objects
        elsif( ref($_[0]) eq $class ) {
          scalar(@_) == 1 ?
            $query = new2obj($op, @_) :
              $query = newXobj($op, @_);
        }
        else {
          Carp::carp( "all of new()'s arguments after the first must be search terms (scalars), or $class objects" );
          $invalid_args++;
        }
      }
    }
  }
  if( $invalid_args ) {
    Carp::carp( "USAGE: $class->new('term'), $class->new(OP, \@terms) or $class->new(OP, \@queries)" );
    exit;
  }
  bless $query, $class;
  return $query;
}

sub new_term {
  my $class = shift;
  my $query;

  if (@_ < 1 or @_ > 3) {
    Carp::carp( "new_term takes 1, 2 or 3 arguments only" );
  }
  my ($term, $wqf, $pos) = @_;
  $wqf = 1 unless defined $wqf;
  $pos = 0 unless defined $pos;

  $query = new1weight($term, $wqf, $pos);

  bless $query, $class;
  return $query;
}

sub _all_equal {
  my $first = shift;
  while(@_) {
    return 0 if $first ne shift;
  }
  return 1;
}

sub get_terms {
    my $self = shift;
    my @terms;
    my $q=$self->get_terms_begin;
    while ($q ne $self->get_terms_end) {
        push @terms,$q->get_termname;
        $q++;
    }
    return @terms;
}

1;
