use Test::More;
BEGIN { plan tests => 20 };
use Search::Xapian qw(:all);

package main;

my $perlSpy_destroy_count = 0;
sub inc_perlSpy_destroy_count {
  $perlSpy_destroy_count++;
}

my $db = Search::Xapian::WritableDatabase->new();

{

  my $i = 0;
  for my $q (qw( low high high )) {
    my $doc = Search::Xapian::Document->new();
    $doc->set_data("test t$i $q");
    $doc->add_posting("test", 0);
    $doc->add_posting("t$i", 1);
    $doc->add_posting($q, 2);
    $doc->add_value(0, $q);
    $db->add_document($doc);
    $i++;
  }

  my $spy;
  ok( $spy = Search::Xapian::ValueCountMatchSpy->new(0), "ValueCountMatchSpy created ok" );

  my $enq;
  ok( $enq = $db->enquire('test'), "db enquirable" );

  $enq->add_matchspy($spy);

  is( $spy->name(), "Xapian::ValueCountMatchSpy", "match spy corretly identified as Xapian::ValueCountMatchSpy" );

  my $mset = $enq->get_mset(0, 10, 10000);

  print $spy->get_description() . "\n";

  is( $spy->get_total(), 3, "match spy went through correct number of documents" );

  my $ref = [];
  $ref->[0] = { name => "high", freq => 2 };
  $ref->[1] = { name => "low" , freq => 1 };

  my $it;
  ok( $it = $spy->values_begin(), "values iterator properly returned from match spy" );

  $i = 0;
  for (my $it = $spy->values_begin(); $it != $spy->values_end(); $it++) {
    is( $it->get_termname(), $ref->[$i]->{name}, "Term iterator index $i was '" . $ref->[$i]->{name} . "' as expected" );
    is( $it->get_termfreq(), $ref->[$i]->{freq}, "Correct frequency for index $i");
    $i++;
  }

  ok( $it = $spy->top_values_begin(1), "top values iterator properly returned from match spy" );
  is( $it->get_termname(), $ref->[0]->{name}, "top value is indeed '" . $ref->[0]->{name} . "' as returned by top values iterator" );

  $enq->clear_matchspies();

  {
    my $perlSpy = ExamplePerlMatchSpy->new(0);
    $enq->add_matchspy($perlSpy);
    $enq->get_mset(0, 10, 10000);

    is( $spy->get_total(), 3, "clear matchspies properly unhooked ValueCountMatchSpy" );

    is( $perlSpy->get_total(), 3, "ExamplePerlMatchSpy registered 3 documents" );

    my $values = $perlSpy->get_values();
    foreach my $val (@$ref) {
      is($values->{$val->{name}}, $val->{freq}, "TestPerlMatchSpy properly recorded value \"" . $val->{name} . "\" with frequency " . $val->{freq}); 
    }
  }

  is($perlSpy_destroy_count, 0, "ExamplePerlMatchSpy not yet destroyed since Enquiry is still in lexical scope");
}

is($perlSpy_destroy_count, 1, "ExamplePerlMatchSpy properly destroyed");

package ExamplePerlMatchSpy;

  use Search::Xapian qw(:all);
  use parent 'Search::Xapian::PerlMatchSpy';
  use fields qw(total valueno values _destroy_events);
  use Test::More;

  sub DESTROY {
    main->inc_perlSpy_destroy_count();
  }

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
    ok($wt > 0.068992 && $wt < 0.068994, "Weight properly registered");
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
1;

