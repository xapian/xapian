# Before 'make install' is performed this script should be runnable with
# 'make test'. After 'make install' it should work as 'perl test.pl'

# FIXME: these tests pass in the XS version.
my $disable_fixme = 1;

#########################

# Make warnings fatal
use warnings;
BEGIN {$SIG{__WARN__} = sub { die "Terminating test due to warning: $_[0]" } };

use Test;
use Devel::Peek;
BEGIN { plan tests => 61 };
use Xapian qw(:standard);
ok(1); # If we made it this far, we're ok.

#########################

# Insert your test code below, the Test module is use()ed here so read
# its man page ( perldoc Test ) for help writing this test script.

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
ok( $query->get_description(), 'Query((one@1 OR two@2))' );

ok( $query = $qp->parse_query( 'one OR (two AND three)' ) );
ok( $query->get_description(), 'Query((one@1 OR (two@2 AND three@3)))' );

ok( my $enq = $database->enquire( $query ) );

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
  ok( $qp->set_stopper($stopper), undef );
}
ok( $qp->parse_query("one two many") );

$qp = new Xapian::QueryParser();
my $vrp;
ok( $vrp = new Xapian::StringValueRangeProcessor(1) );
$qp->add_valuerangeprocessor($vrp);
$qp->add_boolean_prefix("test", "XTEST");

my $pair;
foreach $pair (
    [ 'a..b', '0 * VALUE_RANGE 1 a b' ],
    [ '$50..100', '0 * VALUE_RANGE 1 $50 100' ],
    [ '$50..$99', '0 * VALUE_RANGE 1 $50 $99' ],
    [ '$50..$100', '' ],
    [ '02/03/1979..10/12/1980', '0 * VALUE_RANGE 1 02/03/1979 10/12/1980' ],
    [ 'a..b hello', '(hello@1 FILTER VALUE_RANGE 1 a b)' ],
    [ 'hello a..b', '(hello@1 FILTER VALUE_RANGE 1 a b)' ],
    [ 'hello a..b world', '((hello@1 OR world@2) FILTER VALUE_RANGE 1 a b)' ],
    [ 'hello a..b test:foo', '(hello@1 FILTER (VALUE_RANGE 1 a b AND XTESTfoo))' ],
    [ '-5..7', '0 * VALUE_RANGE 1 -5 7' ],
    [ 'hello -5..7', '(hello@1 FILTER VALUE_RANGE 1 -5 7)' ],
    [ '-5..7 hello', '(hello@1 FILTER VALUE_RANGE 1 -5 7)' ],
    [ '"time flies" 09:00..12:30', '((time@1 PHRASE 2 flies@2) FILTER VALUE_RANGE 1 09:00 12:30)' ]
    ) {
    my ($str, $res) = @{$pair};
    my $query = $qp->parse_query($str);
    ok( $query->get_description(), "Query($res)" );
}

$qp = new Xapian::QueryParser();

my $vrp1 = new Xapian::DateValueRangeProcessor(1);
my $vrp2 = new Xapian::NumberValueRangeProcessor(2);
my $vrp3 = new Xapian::StringValueRangeProcessor(3);
my $vrp4 = new Xapian::NumberValueRangeProcessor(4, '$');
my $vrp5 = new Xapian::NumberValueRangeProcessor(5, 'kg', 0);
my $vrp6 = new Xapian::StringValueRangeProcessor(6, 'country:');
my $vrp7 = new Xapian::StringValueRangeProcessor(7, ':name', 0);
$qp->add_valuerangeprocessor( $vrp1 );
$qp->add_valuerangeprocessor( $vrp2 );
$qp->add_valuerangeprocessor( $vrp4 );
$qp->add_valuerangeprocessor( $vrp5 );
$qp->add_valuerangeprocessor( $vrp6 );
$qp->add_valuerangeprocessor( $vrp7 );
$qp->add_valuerangeprocessor( $vrp3 );

$qp->add_boolean_prefix("test", "XTEST");
foreach $pair (
    [ 'a..b', '0 * VALUE_RANGE 3 a b' ],
    [ '1..12', "0 * VALUE_RANGE 2 \\xa0 \\xae" ],
    [ '20070201..20070228', '0 * VALUE_RANGE 1 20070201 20070228' ],
    [ '$10..20', "0 * VALUE_RANGE 4 \\xad \\xb1" ],
    [ '$10..$20', "0 * VALUE_RANGE 4 \\xad \\xb1" ],
    [ '12..42kg', "0 * VALUE_RANGE 5 \\xae \\xb5\@" ],
    [ '12kg..42kg', "0 * VALUE_RANGE 5 \\xae \\xb5\@" ],
    [ '12kg..42', '0 * VALUE_RANGE 3 12kg 42' ],
    [ '10..$20', '' ],
    [ '1999-03-12..2020-12-30', '0 * VALUE_RANGE 1 19990312 20201230' ],
    [ '1999/03/12..2020/12/30', '0 * VALUE_RANGE 1 19990312 20201230' ],
    [ '1999.03.12..2020.12.30', '0 * VALUE_RANGE 1 19990312 20201230' ],
    [ '12/03/99..12/04/01', '0 * VALUE_RANGE 1 19990312 20010412' ],
    [ '03-12-99..04-14-01', '0 * VALUE_RANGE 1 19990312 20010414' ],
    [ '(test:a..test:b hello)', '(hello@1 FILTER VALUE_RANGE 3 test:a test:b)' ],
    [ 'country:chile..denmark', '0 * VALUE_RANGE 6 chile denmark' ],
    [ 'albert..xeni:name', '0 * VALUE_RANGE 7 albert xeni' ],
    ) {
    my ($str, $res) = @{$pair};
    my $query = $qp->parse_query($str);
    ok( $query->get_description(), "Query($res)" );
}

$qp = new Xapian::QueryParser();

{
  my $vrpdate = new Xapian::DateValueRangeProcessor(1, 1, 1960);
  $qp->add_valuerangeprocessor( $vrpdate );
}

foreach $pair (
    [ '12/03/99..12/04/01', '0 * VALUE_RANGE 1 19991203 20011204' ],
    [ '03-12-99..04-14-01', '0 * VALUE_RANGE 1 19990312 20010414' ],
    [ '01/30/60..02/02/59', '0 * VALUE_RANGE 1 19600130 20590202' ],
    ) {
    my ($str, $res) = @{$pair};
    my $query = $qp->parse_query($str);
    ok( $query->get_description(), "Query($res)" );
}

# Regression test for Xapian bug fixed in 1.0.5.0.  In 1.0.0.0-1.0.4.0
# we tried to catch const char * not Xapian::Error, so std::terminate got
# called.
$qp = Xapian::QueryParser->new;
eval {
    $qp->parse_query('other* AND', FLAG_BOOLEAN|FLAG_WILDCARD);
};
ok($@);
ok(ref($@), "Xapian::QueryParserError", "correct class for exception");
ok($@->isa('Xapian::Error'));
ok($@->get_msg, "Syntax: <expression> AND <expression>", "get_msg works");
ok( $disable_fixme || $@ =~ /^Exception: Syntax: <expression> AND <expression>(?: at \S+ line \d+\.)?$/ );

# Check FLAG_DEFAULT is wrapped (new in 1.0.11.0).
ok( $qp->parse_query('hello world', FLAG_DEFAULT|FLAG_BOOLEAN_ANY_CASE) );

1;
