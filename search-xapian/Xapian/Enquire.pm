package Search::Xapian::Enquire;

use 5.006;
use strict;
use warnings;
use Carp;

use Search::Xapian::MSet::Tied;

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

sub set_query {
  my $self = shift;
  my $query = shift;
  if( ref( $query ) ne 'Search::Xapian::Query' ) {
    $query = Search::Xapian::Query->new( $query, @_ );
  }
  $self->set_query_object( $query );
}

sub matches {
  my ($self, $start, $size) = @_;
  my @array;
  tie( @array, 'Search::Xapian::MSet::Tied', $self->get_mset($start, $size) );
  return @array;
}

sub get_matching_terms_begin {
  my $self = shift;
  if( scalar(@_) == 1 ) {
    my $arg = shift;
    my $arg_class = ref( $arg );
    if( $arg_class eq 'Search::Xapian::MSetIterator' ) {
      return $self->get_matching_terms_begin1($arg);
    } else {
      return $self->get_matching_terms_begin2($arg);
    }
  }
  Carp::carp( "USAGE: \$enquire->get_matching_terms_begin(\$docid) or \$enquire->get_matching_terms_begin(\$msetiterator)" );
  exit;
}

sub get_matching_terms_end {
  my $self = shift;
  if( scalar(@_) == 1 ) {
    my $arg = shift;
    my $arg_class = ref( $arg );
    if( $arg_class eq 'Search::Xapian::MSetIterator' ) {
      return $self->get_matching_terms_end1($arg);
    } else {
      return $self->get_matching_terms_end2($arg);
    }
  }
  Carp::carp( "USAGE: \$enquire->get_matching_terms_end(\$docid) or \$enquire->get_matching_terms_end(\$msetiterator)" );
  exit;
}

1;
