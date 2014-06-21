# Before 'make install' is performed this script should be runnable with
# 'make test'. After 'make install' it should work as 'perl test.pl'

#########################

# change 'tests => 1' to 'tests => last_test_to_print';

# Make warnings fatal
use warnings;
BEGIN {$SIG{__WARN__} = sub { die "Terminating test due to warning: $_[0]" } };

use Test;
use Devel::Peek;
BEGIN { plan tests => 6 };
use Xapian qw(:standard);
ok(1); # If we made it this far, we're ok.

#########################

# Insert your test code below, the Test module is use()ed here so read
# its man page ( perldoc Test ) for help writing this test script.

# first create database dir, if it doesn't exist;
my $db_dir = 'testdb-exception-modified';

if( (! -e $db_dir) or (! -d $db_dir) ) {
  mkdir( $db_dir );
}

opendir( DB_DIR, $db_dir );
while( defined( my $file = readdir( DB_DIR ) ) ) {
  next if $file =~ /^\.+$/;
  unlink( "$db_dir/$file" ) or die "Could not delete '$db_dir/$file': $!";
}
closedir( DB_DIR );

my $create = Xapian::WritableDatabase->new( $db_dir, Xapian::DB_CREATE );

$create = undef;

my $read = Xapian::Database->new( $db_dir );

my $write = Xapian::WritableDatabase->new( $db_dir, Xapian::DB_CREATE_OR_OPEN );

my $enq = $read->enquire(OP_OR, "test");

# Let's try to index something.
my $term = 'test';

my $docid;
for my $num (1..1000) {
  my $doc = Xapian::Document->new();

  $doc->set_data( "$term $num" );

  $doc->add_posting( $term, 0 );
  $doc->add_posting( $num, 1 );

  $doc->add_value(0, $num);
  $write->add_document( $doc );
}
$write->flush();
$read->reopen();

for my $num (qw(three four five)) {
  my $doc = Xapian::Document->new();

  $doc->set_data( "$term $num" );

  $doc->add_posting( $term, 0 );
  $doc->add_posting( $num, 1 );

  $doc->add_value(0, $num);
  $write->add_document( $doc );
  $write->flush();
}
$write->flush();
eval {
    my $mset = $enq->get_mset(0, 10);
};
ok($@);
ok(ref($@), "Xapian::DatabaseModifiedError", "correct class for exception");
ok($@->isa('Xapian::Error'));

ok($@->get_msg, "The revision being read has been discarded - you should call Xapian::Database::reopen() and retry the operation", "get_msg works");

# WritableDatabase::reopen() is a no-op, but it shouldn't fail.
$write->reopen();
ok(1);

1;
