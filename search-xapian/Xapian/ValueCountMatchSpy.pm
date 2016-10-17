package Search::Xapian::ValueCountMatchSpy;

use 5.006;
use strict;
use warnings;
use Carp;

use Search::Xapian::MatchSpy;

require DynaLoader;

our @ISA = qw( DynaLoader Search::Xapian::MatchSpy );

# In a new thread, copy objects of this class to unblessed, undef values.
sub CLONE_SKIP { 1 }

sub new {
  my $class = shift;
  my $matcher;

  if (scalar(@_) == 0) {
    $matcher = new1();
  } elsif (scalar(@_) == 1) {
    $matcher = new2(@_);
  } else {
    Carp::carp("USAGE: $class->new(), $class->new(slot)");
    exit;
  }
  bless $matcher, $class;
  return $matcher;
}

1;

__END__

=head1 NAME

Search::Xapian::ValueCountMatchSpy - Class for counting the frequencies of values in the matching documents.

=head1 SYNOPSIS

  use Search::Xapian qw(:all);

  my $db = Search::Xapian::Database->new( '[DATABASE DIR]' );
  my $enq = $db->enquire( '[QUERY TERM]' );
  my $spy = Search::Xapian::ValueCountMatchSpy->new(0);
  $enq->add_matchspy($spy);
  my $mset = $enq->get_mset(0, 10, 10000);

  print "Match spy registered " . $spy->get_total() . " documents\n";

  my $end = $spy->values_end();
  for (my $it = $spy->values_begin(); $it != $end; $it++) {
    print $it->get_termname() . " - " . $it->get_termfreq();
  }

=head1 DESCRIPTION

Class for counting the frequencies of values in the matching documents.
Wraps Xapian::ValueCountMatchSpy

=head1 METHODS

=over 4

=item new

Constructor. Either takes no parameters, or a single valueno parameter identifying the slot.

=item get_total

Return the total number of documents tallied.

=item values_begin

Get an iterator over the values seen in the slot.

=item values_end

End iterator corresponding to values_begin().

=item top_values_begin <maxvalues>

Get an iterator over the most frequent values seen in the slot.

=item top_values_end <maxvalues>

End iterator corresponding to top_values_begin().

=back

=head1 SEE ALSO

L<Search::Xapian>,
L<Search::Xapian::MatchSpy>

=cut
