package Search::Xapian::QueryParser;

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

# TODO: Add hash-based initialization
sub new() {
  my $class = shift;
  my $qp = new0();
  
  bless $qp, $class;
  $qp->set_database(@_) if scalar(@_) == 1;

  return $qp;
}

1;

__END__

=head1 NAME

Search::Xapian::QueryParser - Parse a query string into a Search::Xapian::Query object

=head1 DESCRIPTION

This module turns a human readable string into something Xapian can
understand.  The syntax supported is designed to be similar to other web
based search engines, so that users familiar with them don't have to learn
a whole new syntax.

=head1 SYNOPSIS

  use Search::Xapian qw/:standard/;

  my $qp = new Search::Xapian::QueryParser( [$database] );
  $qp->set_stemming_options("english",1); # Replace 1 with 0 if you want to disable stemming
  $qp->set_default_op(OP_AND);

  $database->enquire($qp->parse_string('a word OR two NEAR "a phrase" NOT (too difficult) +eh'));
  
=head1 REFERENCE

  http://www.xapian.org/docs/queryparser.html
  http://www.xapian.org/docs/sourcedoc/html/classXapian_1_1QueryParser.html

=head1 TODO

Implement Xapian::Stopper.

=cut
