use strict;
# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl test.pl'

#########################

# change 'tests => 1' to 'tests => last_test_to_print';

use Test;
use Devel::Peek;
BEGIN { plan tests => 6 };
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

# Check that our exception list includes exceptions new in 1.4.x.
# Regression test for bug fixed in 1.2.25.3.
my $doc = Search::Xapian::Document->new();
$doc->add_term("foo");
$doc->add_term("food");
$database->add_document($doc);
my $qp = Search::Xapian::QueryParser->new();
$qp->set_max_wildcard_expansion(1);
my $enquire = Search::Xapian::Enquire->new($database);
eval {
    my $query = $qp->parse_query('fo*', FLAG_WILDCARD);
    $enquire->set_query($query);
    my $mset = $enquire->get_mset(0, 10);
};
ok( $@ );
ok( $@ !~ /something terrible happened/);

1;
