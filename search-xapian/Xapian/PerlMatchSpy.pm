package Search::Xapian::PerlMatchSpy;

use 5.006;
use strict;
use warnings;
use Carp;

use Search::Xapian::MatchSpy;
require DynaLoader;
@Search::Xapian::PerlMatchSpy = qw(DynaLoader Search::Xapian::MatchSpy);

sub CLONE_SKIP { 1 }

sub register {
  die "Register is virtual in PerlMatchSpy - provide implementation in sub-class";
}

1;

__END__

=head1 NAME

Search::Xapian::PerlMatchSpy - Class for providing Perl implementations of MatchSpy.

=head1 SYNOPSIS


  use Search::Xapian qw(:all);

  package main;

  my $db = Search::Xapian::Database->new( '[DATABASE DIR]' );
  my $enq = $db->enquire( '[QUERY TERM]' );
  my $spy = new ExamplePerlMatchSpy(0);
  $enq->add_matchspy($spy);
  my $mset = $enq->get_mset(0, 10, 10000);

  print "Perl match spy registered " . $spy->get_total() . " documents\n";
  my $values = $spy->get_values();

  package ExamplePerlMatchSpy;

  use Search::Xapian qw(:all);
  use parent 'Search::Xapian::PerlMatchSpy';
  use fields qw(total valueno values);

  sub new {
    my ($class, $valueno) = @_;
    my $self = fields::new($class);
    $self->{valueno} = $valueno;
    $self->{values} = {};
    $self->{total} = 0;
    return $self;
  }

  sub register {
    my ($self, $doc, $wt) = @_;
    my $val = $doc->get_value($self->{valueno});
    my $values = $self->{values};
    $values->{$val} = 0 unless $values->{$val};
    $values->{$val}++;
    $self->{total}++;
  }

  sub get_total {
    return shift->{total};
  }

  sub get_values {
    return shift->{values};
  }

=head1 DESCRIPTION

Class for providing Perl implementations of MatchSpy.

Sub-classes need to use this parent class and provide implementations for the register sub.

=head1 METHODS

=over 4 

=item register <document> <weight>

Virtual method. Must be implemented by extending classes. Registers a document with the match spy.

This is called by the matcher once with each document seen by the matcher
during the match process. Note that the matcher will often not see all the
documents which match the query, due to optimisations which allow low-weighted
documents to be skipped, and allow the match process to be terminated early.

=back

=head1 SEE ALSO

L<Search::Xapian>,L<Search::Xapian::Enquire>,L<Search::Xapian::MatchSpy>

=cut
