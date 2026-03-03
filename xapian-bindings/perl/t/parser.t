use strict;
# Before 'make install' is performed this script should be runnable with
# 'make test'. After 'make install' it should work as 'perl test.pl'

#########################

# Make warnings fatal
use warnings;
BEGIN {$SIG{__WARN__} = sub { die "Terminating test due to warning: $_[0]" } };

use Test::More;
use Devel::Peek;
BEGIN { plan tests => 76 };
use Xapian qw(:standard);
ok(1); # If we made it this far, we're ok.

#########################

# Insert your test code below, the Test module is use()ed here so read
# its man page ( perldoc Test ) for help writing this test script.

sub mset_expect_order (\@@) {
    my ($m, @a) = @_;
    my @m = map { $_->get_docid() } @{$m};
    is( scalar @m, scalar @a );
    for my $j (0 .. (scalar @a - 1)) {
	is( $m[$j], $a[$j] );
    }
}

# first create database dir, if it doesn't exist;
my $db_dir = 'testdb';

my $database;
ok( $database = Xapian::Database->new( $db_dir ) );

my $qp = new Xapian::QueryParser( $database );
$qp = new Xapian::QueryParser();

$qp->set_stemmer( Xapian::Stem->new('english') );
$qp->set_stemming_strategy( STEM_ALL );
$qp->set_default_op( OP_AND );

my $query;
ok( $query = $qp->parse_query( 'one or two', FLAG_BOOLEAN|FLAG_BOOLEAN_ANY_CASE|FLAG_SPELLING_CORRECTION ) );
ok( not $qp->get_corrected_query_string());
is( $query->get_description(), 'Query((one@1 OR two@2))' );

ok( $query = $qp->parse_query( 'one OR (two AND three)' ) );
is( $query->get_description(), 'Query((one@1 OR (two@2 AND three@3)))' );

ok( my $enq = $database->enquire( $query ) );

{
  is( $qp->set_stopper(sub { $_[0] eq 'bad' }), undef );
  is( $qp->parse_query("bad news")->get_description(), 'Query(news@2)' );
}

{
  my @stopwords = qw(a the in on and);
  my $stopper;
  ok( $stopper = new Xapian::SimpleStopper(@stopwords) );
  foreach (@stopwords) {
    ok( $stopper->stop_word($_) );
  }
  foreach (qw(one two three four five)) {
    ok( !$stopper->stop_word($_) );
  }
  is( $qp->set_stopper($stopper), undef );
}
ok( $qp->parse_query("one two many") );

$qp = new Xapian::QueryParser();
my $rp;
ok( $rp = new Xapian::RangeProcessor(1) );
$qp->add_rangeprocessor($rp);
$qp->add_boolean_prefix("test", "XTEST");

my $pair;
foreach $pair (
    [ 'a..b', 'VALUE_RANGE 1 a b' ],
    [ '$50..100', 'VALUE_RANGE 1 $50 100' ],
    [ '$50..$99', 'VALUE_RANGE 1 $50 $99' ],
    [ '$50..$100', '' ],
    [ '02/03/1979..10/12/1980', 'VALUE_RANGE 1 02/03/1979 10/12/1980' ],
    [ 'a..b hello', '(hello@1 FILTER VALUE_RANGE 1 a b)' ],
    [ 'hello a..b', '(hello@1 FILTER VALUE_RANGE 1 a b)' ],
    [ 'hello a..b world', '((hello@1 OR world@2) FILTER VALUE_RANGE 1 a b)' ],
    [ 'hello a..b test:foo', '(hello@1 FILTER (VALUE_RANGE 1 a b AND XTESTfoo))' ],
    [ '-5..7', 'VALUE_RANGE 1 -5 7' ],
    [ 'hello -5..7', '(hello@1 FILTER VALUE_RANGE 1 -5 7)' ],
    [ '-5..7 hello', '(hello@1 FILTER VALUE_RANGE 1 -5 7)' ],
    [ '"time flies" 09:00..12:30', '((time@1 PHRASE 2 flies@2) FILTER VALUE_RANGE 1 09:00 12:30)' ]
    ) {
    my ($str, $res) = @{$pair};
    my $query = $qp->parse_query($str);
    is( $query->get_description(), "Query($res)" );
}

$qp = new Xapian::QueryParser();

my $rp1 = new Xapian::DateRangeProcessor(1);
my $rp2 = new Xapian::NumberRangeProcessor(2);
my $rp3 = new Xapian::RangeProcessor(3);
my $rp4 = new Xapian::NumberRangeProcessor(4, '$', Xapian::RP_REPEATED);
my $rp5 = new Xapian::NumberRangeProcessor(5, 'kg', Xapian::RP_REPEATED|Xapian::RP_SUFFIX);
my $rp6 = new Xapian::RangeProcessor(6, 'country:');
my $rp7 = new Xapian::RangeProcessor(7, ':name', Xapian::RP_SUFFIX);
$qp->add_rangeprocessor( $rp1 );
$qp->add_rangeprocessor( $rp2 );
$qp->add_rangeprocessor( $rp4 );
$qp->add_rangeprocessor( $rp5 );
$qp->add_rangeprocessor( $rp6 );
$qp->add_rangeprocessor( $rp7 );
$qp->add_rangeprocessor( $rp3 );

$qp->add_boolean_prefix("test", "XTEST");

foreach $pair (
    [ 'a..b', 'VALUE_RANGE 3 a b' ],
    [ '1..12', "VALUE_RANGE 2 \\xa0 \\xae" ],
    [ '20070201..20070228', 'VALUE_RANGE 1 20070201 20070228' ],
    [ '$10..20', "VALUE_RANGE 4 \\xad \\xb1" ],
    [ '$10..$20', "VALUE_RANGE 4 \\xad \\xb1" ],
    [ '12..42kg', "VALUE_RANGE 5 \\xae \\xb5\@" ],
    [ '12kg..42kg', "VALUE_RANGE 5 \\xae \\xb5\@" ],
    [ '12kg..42', 'VALUE_RANGE 3 12kg 42' ],
    [ '10..$20', '' ],
    [ '1999-03-12..2020-12-30', 'VALUE_RANGE 1 19990312 20201230' ],
    [ '1999/03/12..2020/12/30', 'VALUE_RANGE 1 19990312 20201230' ],
    [ '1999.03.12..2020.12.30', 'VALUE_RANGE 1 19990312 20201230' ],
    [ '12/03/99..12/04/01', 'VALUE_RANGE 1 19990312 20010412' ],
    [ '03-12-99..04-14-01', 'VALUE_RANGE 1 19990312 20010414' ],
    [ '(test:a..test:b hello)', '(hello@1 FILTER VALUE_RANGE 3 test:a test:b)' ],
    [ 'country:chile..denmark', 'VALUE_RANGE 6 chile denmark' ],
    [ 'albert..xeni:name', 'VALUE_RANGE 7 albert xeni' ],
    ) {
    my ($str, $res) = @{$pair};
    my $query = $qp->parse_query($str);
    is( $query->get_description(), "Query($res)" );
}

$qp = new Xapian::QueryParser();
$qp->add_rangeprocessor( sub { Xapian::Query->new("spam") } );
foreach $pair (
    [ '123..345', '0 * spam' ],
    ) {
    my ($str, $res) = @{$pair};
    my $query = $qp->parse_query($str);
    is( $query->get_description(), "Query($res)" );
}

$qp = new Xapian::QueryParser();
{
  my $rpdate = new Xapian::DateRangeProcessor(1, Xapian::RP_DATE_PREFER_MDY, 1960);
  $qp->add_rangeprocessor( $rpdate );
}

foreach $pair (
    [ '12/03/99..12/04/01', 'VALUE_RANGE 1 19991203 20011204' ],
    [ '03-12-99..04-14-01', 'VALUE_RANGE 1 19990312 20010414' ],
    [ '01/30/60..02/02/59', 'VALUE_RANGE 1 19600130 20590202' ],
    ) {
    my ($str, $res) = @{$pair};
    my $query = $qp->parse_query($str);
    is( $query->get_description(), "Query($res)" );
}

$qp = new Xapian::QueryParser();
$qp->add_prefix("foo", sub {return new Xapian::Query('foo')});
is( $qp->parse_query("foo:test")->get_description(), 'Query(foo)' );

# Regression test for Search::Xapian bug fixed in 1.0.5.0.  In 1.0.0.0-1.0.4.0
# we tried to catch const char * not Xapian::Error, so std::terminate got
# called.
$qp = Xapian::QueryParser->new;
eval {
    $qp->parse_query('other* AND', FLAG_BOOLEAN|FLAG_WILDCARD);
};
ok($@);
is(ref($@), "Xapian::QueryParserError", "correct class for exception");
ok($@->isa('Xapian::Error'));
is($@->get_msg, "Syntax: <expression> AND <expression>", "get_msg works");

# Check FLAG_DEFAULT is wrapped (new in 1.0.11.0).
ok( $qp->parse_query('hello world', FLAG_DEFAULT|FLAG_BOOLEAN_ANY_CASE) );

# Test OP_WILDCARD with limits.
my ($q, @matches);
ok( $enq = Xapian::Enquire->new($database) );

$qp->set_max_expansion(1, Xapian::WILDCARD_LIMIT_FIRST);
ok( $q = $qp->parse_query('t*', FLAG_WILDCARD) );
$enq->set_query($q);
@matches = $enq->matches(0, 10);
mset_expect_order(@matches, (1, 2));

$qp->set_max_expansion(1, Xapian::WILDCARD_LIMIT_MOST_FREQUENT);
ok( $q = $qp->parse_query('t*', FLAG_WILDCARD) );
$enq->set_query($q);
@matches = $enq->matches(0, 10);
mset_expect_order(@matches, (1, 2));

$qp->set_max_expansion(1, Xapian::WILDCARD_LIMIT_ERROR);
ok( $q = $qp->parse_query('t*', FLAG_WILDCARD) );
$enq->set_query($q);
eval {
    @matches = $enq->matches(0, 10);
};
ok( $@ );
is(ref($@), "Xapian::WildcardError", "correct class for exception");

1;
