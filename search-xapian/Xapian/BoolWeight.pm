package Search::Xapian::BoolWeight;

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
  } else {
    Carp::carp("USAGE: $class->new()");
    exit;
  }
  bless $weight, $class;
  return $weight;
}

1;

__END__

=head1 NAME

Search::Xapian::BoolWeight - Boolean Weighting scheme.

=head1 DESCRIPTION

Boolean Weighting scheme. All documents get a weight of 0.

=head1 METHODS

=over 4

=item new

Constructor. Takes no arguments.

=back

=head1 SEE ALSO

L<Search::Xapian>,L<Search::Xapian::Enquire>

=cut
