# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl test.pl'

#########################

use Test::More;
BEGIN { plan tests => 4 };
use Search::Xapian qw(:standard);

ok(1); # If we made it this far, we're ok.

#########################

my $db_dir = 'testdb';

# Delete contents of database dir, if it exists.
if (opendir( DB_DIR, $db_dir )) {
  while( defined( my $file = readdir( DB_DIR ) ) ) {
    next if $file =~ /^\.+$/;
    unlink( "$db_dir/$file" ) or die "Could not delete '$db_dir/$file': $!";
  }
  closedir( DB_DIR );
}

is( $Search::Xapian::DB_NAMES[Search::Xapian::DB_CREATE], "DB_CREATE" );

my $database;
ok( $database = Search::Xapian::WritableDatabase->new( $db_dir, Search::Xapian::DB_CREATE ) );
ok( $database = Search::Xapian::WritableDatabase->new() );

1;
