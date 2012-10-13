package Search::Xapian::Query;

use 5.006;
use strict;
use warnings;
use Carp;

require DynaLoader;

our @ISA = qw(DynaLoader);

# Preloaded methods go here.

# In a new thread, copy objects of this class to unblessed, undef values.
sub CLONE_SKIP { 1 }

use overload '""' => sub { $_[0]->get_description() }, # FIXME: perhaps unwise?
             'fallback' => 1;

sub new {
  my $class = shift;
  my $query;

  if( @_ == 0 ) {
    $query = new0();
  } elsif( @_ == 1 ) {
    $query = new1(@_);
  } else {
    my $op = $_[0];
    if( $op !~ /^\d+$/ ) {
	Carp::croak( "USAGE: $class->new('term') or $class->new(OP, <args>)" );
    }
    if( $op == 8 ) { # FIXME: 8 is OP_VALUE_RANGE; eliminate hardcoded literal
      if( @_ != 4 ) {
	Carp::croak( "USAGE: $class->new(OP_VALUE_RANGE, VALNO, START, END)" );
      }
      $query = new4range( @_ );
    } elsif( $op == 9 ) { # FIXME: OP_SCALE_WEIGHT
      if( @_ != 3 ) {
        Carp::croak( "USAGE: $class->new(OP_SCALE_WEIGHT, QUERY, FACTOR)" );
      }
      $query = new3scale( @_ );
    } elsif( $op == 11 || $op == 12 ) { # FIXME: OP_VALUE_GE, OP_VALUE_LE; eliminate hardcoded literals
      if( @_ != 3 ) {
        Carp::croak( "USAGE: $class->new(OP_VALUE_[GL]E, VALNO, LIMIT)" );
      }
      $query = new3range( @_ );
    } else {
      $query = newN( @_ );
    }
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

sub MatchNothing () { Search::Xapian::Query->new }

sub MatchAll () { Search::Xapian::Query->new('') }

1;
