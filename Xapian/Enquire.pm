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
    $self->set_query1( $query );
    return;
  }
  my $nargs = scalar(@_);
  if( $nargs > 1) {
    Carp::carp( "USAGE: \$enquire->set_query(\$query) or \$enquire->set_query(\$query, \$length)" );
    exit;
  }
  if( $nargs == 0 ) {
    $self->set_query1( $query );
  } else {
    $self->set_query2( $query, shift );
  }
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
      return $self->get_matching_terms_begin2($arg);
    } else {
      return $self->get_matching_terms_begin1($arg);
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
      return $self->get_matching_terms_end2($arg);
    } else {
      return $self->get_matching_terms_end1($arg);
    }
  }
  Carp::carp( "USAGE: \$enquire->get_matching_terms_end(\$docid) or \$enquire->get_matching_terms_end(\$msetiterator)" );
  exit;
}

=head1 NAME

Search::Xapian::Enquire - Make queries against a database

=head1 DESCRIPTION

This class provides an interface to the information retrieval system for the 
purpose of searching.

=head1 METHODS

=over 4

=item new

=item set_query

takes either a ready made L<Search::Xapian::Query> or a scalar containing
a query, which in that case will be passed to L<Search::Xapian::Query>'s
constructor, together with any other passed arguments.

=item set_query_object <query>

=item get_query

=item matches <start> <size>

Takes the start element, and maximum number of elements, and
returns an array tied to L<Search::Xapian::MSet::Tied>. 

=item get_matching_terms_begin

Returns a L<Search::Xapian::TermIterator>, pointing to the start
of the stream.

=item get_matching_terms_end 

Returns a L<Search::Xapian::TermIterator>, pointing to the end 
of the stream.

=item set_collapse_key <collapse_key>

=item set_sort_forward <sort_forward>

=item set_cutoff <percent_cutoff> [<weight_cutoff>]

=item set_sorting <sort_key> <sort_bands> [<sort_by_relevance>]

=item set_bias <bias_weight>=item set_sorting <sort_key> <sort_bands> [<sort_by_relevance>]

=item set_bias <bias_weight> <bias_halflife>

=item get_mset

Get match set.

=item get_eset

=item register_match_decoder <name> <mdecider>

See xapian api doc for these functions.

=item get_description

Returns a description of the object (for introspection).

=back

=head1 SEE ALSO

L<Search::Xapian::Query>, L<Search::Xapian::Database>

=cut

1;

