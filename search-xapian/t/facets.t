use Test::More;
BEGIN { plan tests => 12 };
use Search::Xapian qw(:all);

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

    note $spy->get_description();

    is( $spy->get_total(), 3, "match spy went through correct number of documents" );

    my $ref = [
        { name => "high", freq => 2 },
        { name => "low" , freq => 1 },
    ];

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
    is( $spy->get_total(), 3, "clearing matchspies doesn't kill our spy reference?" );
}

