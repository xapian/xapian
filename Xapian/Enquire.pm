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

sub AUTOLOAD {
  our $AUTOLOAD;
  if( $AUTOLOAD =~ /^get_matching_terms_(?:begin|end)$/ ) {
    my $self = shift;
    my $invalid_args;
    if( scalar(@_) == 1 ) {
      my $arg = shift;
      my $arg_class = ref( $arg );
      if( $arg_class eq 'Search::Xapian::MSetIterator' ) {
        eval( "\$self->${AUTOLOAD}2(\$arg)" );
      } else {
        eval( "\$self->${AUTOLOAD}1(\$arg)" );
      }
    } else {
      $invalid_args = 1;
    }
    if( $invalid_args ) {
      Carp::carp( "USAGE: \$enquire->$AUTOLOAD(\$docid) or \$enquire->$AUTOLOAD(\$msetiterator)" );
      exit;
    }
  }
}

1;
