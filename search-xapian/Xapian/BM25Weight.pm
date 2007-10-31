package Search::Xapian::BM25Weight;

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
  } elsif (scalar(@_) == 5) {
    $weight = new2(@_);
  } else {
    Carp::carp("USAGE: $class->new(), $class->new(k1, k2, k3, b, min_normlen)");
    exit;
  }
  bless $weight, $class;
  return $weight;
}

1;

__END__

=head1 NAME

Search::Xapian::BM25Weight - BM25 Weighting scheme.

=head1 DESCRIPTION

BM25 Weighting scheme. This is the default weighting scheme.

=head1 METHODS

=over 4 

=item new

Constructor. Either takes no parameters, or the 4 BM25 parameters
(k1, k2, k3, b) and the minimum normalised document length.

=back

=head1 SEE ALSO

L<Search::Xapian>,L<Search::Xapian::Enquire>

=cut
