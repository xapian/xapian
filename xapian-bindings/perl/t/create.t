# Before 'make install' is performed this script should be runnable with
# 'make test'. After 'make install' it should work as 'perl test.pl'

#########################

# Make warnings fatal
use warnings;
BEGIN {$SIG{__WARN__} = sub { die "Terminating test due to warning: $_[0]" } };

use Test::More;
BEGIN { plan tests => 4 };
use Xapian qw(:standard);

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

is( $Xapian::DB_NAMES[Xapian::DB_CREATE], "DB_CREATE" );

my $database;
ok( $database = Xapian::WritableDatabase->new( $db_dir, Xapian::DB_CREATE ) );
ok( $database = Xapian::WritableDatabase->new() );

1;
