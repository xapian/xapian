# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl test.pl'

#########################

# change 'tests => 1' to 'tests => last_test_to_print';

use Test;
use Devel::Peek;
BEGIN { plan tests => 15 };
use Search::Xapian qw(:standard);
ok(1); # If we made it this far, we're ok.

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

my $it = $doc->values_begin();
ok( $it ne $doc->values_end() );
ok( "$it" eq "fudge" );
ok( $it->get_value() eq "fudge" );
ok( $it->get_valueno() == 1 );
++$it;
ok( $it ne $doc->values_end() );
ok( "$it" eq "chocolate" );
ok( $it->get_value() eq "chocolate" );
ok( $it->get_valueno() == 2 );
++$it;
ok( $it eq $doc->values_end() );

$doc->remove_value(1);
ok( $doc->get_value(1) eq "" );
ok( $doc->get_value(2) eq "chocolate" );
$doc->clear_values();
ok( $doc->get_value(2) eq "" );

1;
