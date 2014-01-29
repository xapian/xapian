# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl test.pl'

#########################

# change 'tests => 1' to 'tests => last_test_to_print';

use Test;
use Devel::Peek;
BEGIN { plan tests => 4 };
use Search::Xapian qw(:standard);
ok(1); # If we made it this far, we're ok.

#########################

# Insert your test code below, the Test module is use()ed here so read
# its man page ( perldoc Test ) for help writing this test script.

# first create database dir, if it doesn't exist;
my $db_dir = 'testdb-exception';

if( (! -e $db_dir) or (! -d $db_dir) ) {
  mkdir( $db_dir );
}

opendir( DB_DIR, $db_dir );
while( defined( my $file = readdir( DB_DIR ) ) ) {
  next if $file =~ /^\.+$/;
  unlink( "$db_dir/$file" ) or die "Could not delete '$db_dir/$file': $!";
}
closedir( DB_DIR );

my $database;
ok( $database = Search::Xapian::WritableDatabase->new( $db_dir, Search::Xapian::DB_CREATE ) );
eval {
  # this should work
  my $other_database = Search::Xapian::Database->new( $db_dir );
};
ok( !$@ );
eval {
  # should fail because database is already locked
  my $other_database = Search::Xapian::WritableDatabase->new( $db_dir, Search::Xapian::DB_CREATE );
};
ok( $@ );

1;
