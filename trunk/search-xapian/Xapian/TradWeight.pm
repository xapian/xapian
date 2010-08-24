package Search::Xapian::TradWeight;

use 5.006;
use strict;
use warnings;
use Carp;

use Search::Xapian::Weight;

require DynaLoader;

our @ISA = qw( DynaLoader Search::Xapian::Weight);

# In a new thread, copy objects of this class to unblessed, undef values.
sub CLONE_SKIP { 1 }

sub new {
  my $class = shift;
  my $weight;

  if (scalar(@_) == 0) {
    $weight = new1();
  } elsif (scalar(@_) == 1) {
    $weight = new2(@_);
  } else {
    Carp::carp("USAGE: $class->new(), $class->new(k)");
    exit;
  }
  bless $weight, $class;
  return $weight;
}

1;

__END__

=head1 NAME

Search::Xapian::TradWeight - Traditional Probabilistic Weighting scheme.

=head1 DESCRIPTION

Traditional Probabilistic Weighting scheme, as described by the early papers
on Probabilistic Retrieval.  BM25 generally gives better results.

=head1 METHODS

=over 4 

=item new

Constructor. Either takes no parameters, or a single non-negative parameter k.
If k isn't specified, the default value used is 1.

=back

=head1 SEE ALSO

L<Search::Xapian>,L<Search::Xapian::Enquire>

=cut
