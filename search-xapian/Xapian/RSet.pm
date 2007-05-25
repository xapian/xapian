package Search::Xapian::RSet;

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
  my $rset;
  my $invalid_args;
  if( scalar(@_) == 0 ) {
    $rset = new1();
  } elsif( scalar(@_) == 1 and ref( $_[1] ) eq $class ) {
    $rset = new2(@_);
  } else {
    $invalid_args = 1;
  }
  if( $invalid_args ) {
    Carp::carp( "USAGE: $class->new(), $class->new(\$rset)" );
    exit;
  }
  bless $rset, $class;
  return $rset;
}

sub add_document {
  my $self = shift;
  if( scalar(@_) == 1 ) {
    my $arg = shift;
    my $arg_class = ref( $arg );
    if( $arg_class eq 'Search::Xapian::MSetIterator' ) {
      $self->add_document1($arg);
    } else {
      $self->add_document2($arg);
    }
    return;
  }
  Carp::carp( "USAGE: \$rset->add_document(\$docid) or \$rset->add_document(\$msetiterator)" );
  exit;
}

sub remove_document {
  my $self = shift;
  if( scalar(@_) == 1 ) {
    my $arg = shift;
    my $arg_class = ref( $arg );
    if( $arg_class eq 'Search::Xapian::MSetIterator' ) {
      $self->remove_document1($arg);
    } else {
      $self->remove_document2($arg);
    }
    return;
  }
  Carp::carp( "USAGE: \$rset->remove_document(\$docid) or \$rset->remove_document(\$msetiterator)" );
  exit;
}

sub contains {
  my $self = shift;
  my $invalid_args;
  if( scalar(@_) == 1 ) {
    my $arg = shift;
    my $arg_class = ref( $arg );
    if( $arg_class eq 'Search::Xapian::MSetIterator' ) {
      return $self->contains1($arg);
    } else {
      return $self->contains2($arg);
    }
  }
  Carp::carp( "USAGE: \$rset->contains(\$docid) or \$rset->contains(\$msetiterator)" );
  exit;
}

1;
