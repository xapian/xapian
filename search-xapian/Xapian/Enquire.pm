package Search::Xapian::Enquire;

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
sub AUTOLOAD {
  if( $AUTOLOAD =~ /^get_matching_terms_(?:begin|end)$/ ) {
    my $self = shift;
    my $invalid_args;
    if( scalar(@_) == 1 ) {
      my $arg = shift;
      my $arg_class = shift;
      if( $arg_class eq 'Search::Xapian::MSetIterator' ) {
        $self->"${AUTOLOAD}2"($arg);
      } else {
        $self->"${AUTOLOAD}1"($arg);
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
