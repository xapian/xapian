# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl test.pl'

#########################

# change 'tests => 1' to 'tests => last_test_to_print';

use Test;
use Devel::Peek;
BEGIN { plan tests => 4 };
use Search::Xapian;
ok(1); # If we made it this far, we're ok.

#########################

# Insert your test code below, the Test module is use()ed here so read
# its man page ( perldoc Test ) for help writing this test script.

# first create database dir, if it doesn't exist;
my $db_dir = 'testdb';

if( (! -e $db_dir) or (! -d $db_dir) ) {
  mkdir( $db_dir );
}

my $settings;
ok( $settings = Search::Xapian::Settings->new() );

$settings->set( 'backend', 'auto' );
$settings->set( 'database_create', 'true' );
$settings->set( 'database_allow_overwrite', 'true' );
$settings->set( 'auto_dir', $db_dir );

ok(1);

my $database;
ok( $database = Search::Xapian::Database::Writable->new( $settings ) );

1;
