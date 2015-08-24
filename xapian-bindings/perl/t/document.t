# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl test.pl'

#########################

# change 'tests => 1' to 'tests => last_test_to_print';

use Test;
use Devel::Peek;
BEGIN { plan tests => 26 };
use Search::Xapian qw(:standard);
ok(1); # If we made it this far, we're ok.

# FIXME: these tests pass in the XS version.
my $disable_fixme = 1;

#########################

# Insert your test code below, the Test module is use()ed here so read
# its man page ( perldoc Test ) for help writing this test script.

my $doc = Search::Xapian::Document->new();
$data = "hello world";
$doc->set_data($data);
ok( $doc->get_data() eq $data );
$doc->add_value(1, "fudge");
$doc->add_value(2, "chocolate");
ok( $doc->get_value(1) eq "fudge" );
ok( $doc->get_docid() == 0 );

my $it = $doc->values_begin();
ok( $it ne $doc->values_end() );
ok( $disable_fixme || "$it" eq "fudge" );
ok( $it->get_value() eq "fudge" );
ok( $it->get_valueno() == 1 );
++$it;
ok( $it ne $doc->values_end() );
ok( $disable_fixme || "$it" eq "chocolate" );
ok( $it->get_value() eq "chocolate" );
ok( $it->get_valueno() == 2 );
++$it;
ok( $it eq $doc->values_end() );

$doc->remove_value(1);
ok( $doc->get_value(1) eq "" );
ok( $doc->get_value(2) eq "chocolate" );
$doc->clear_values();
ok( $doc->get_value(2) eq "" );

my $database = Search::Xapian::WritableDatabase->new();

# in <= 0.8.3.0 this added with wdfinc 1
$doc->add_posting( "hello", 1, 100 );
# in <= 0.8.3.0 this added with wdfinc 0
$doc->add_posting( "hello", 2 );
$database->add_document($doc);

ok( $database->get_doclength(1) == 101 );

$doc = Search::Xapian::Document->new();
# in <= 0.8.3.0 this added with wdfinc 1 (happens to work as it should)
$doc->add_posting( "goodbye", 1, 1 );
# in <= 0.8.3.0 this added with wdfinc 1 (happens to work as it should)
$doc->add_posting( "goodbye", 2, 1 );
# in <= 0.8.3.0 this removed with wdfinc 0
$doc->remove_posting( "goodbye", 2 );
$database->add_document($doc);

ok( $database->get_doclength(2) == 1 );

$doc = Search::Xapian::Document->new();
# in <= 0.8.3.0 this added with wdfinc 1
$doc->add_term( "a", 100 );
# in <= 0.8.3.0 this added with wdfinc 0
$doc->add_term( "a" );
$database->add_document($doc);

ok( $database->get_doclength(3) == 101 );

ok( $it = $doc->termlist_begin());
ok( $it ne $doc->termlist_end());
ok( $disable_fixme || "$it" eq "a" );
ok( $it->get_termname() eq "a" );
++$it;
ok( $it eq $doc->termlist_end());

$doc->add_boolean_term( "b" );
$database->add_document($doc);

ok( $database->get_doclength(4) == 101 );

$doc->remove_term( "a" );
$database->add_document($doc);

ok( $database->get_doclength(5) == 0 );

1;
