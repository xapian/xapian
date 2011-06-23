# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl test.pl'

#########################

use Test;
use Devel::Peek;
BEGIN { plan tests => 28 };
use Search::Xapian qw(:standard);
ok(1); # If we made it this far, we're ok.

#########################

# Insert your test code below, the Test module is use()ed here so read
# its man page ( perldoc Test ) for help writing this test script.

# first create database dir, if it doesn't exist;

my $termgen = new Search::Xapian::TermGenerator();
my $doc = new Search::Xapian::Document();
$termgen->set_document($doc);
$termgen->index_text('foo bar baz foo');
$termgen->index_text_without_positions('baz zoo');

my $ti = $doc->termlist_begin();
ok( $ti ne $doc->termlist_end());
ok( $ti->get_termname(), 'bar' );
ok( $ti->get_wdf(), 1 );
my $pi = $ti->positionlist_begin();
ok( $pi ne $ti->positionlist_end() );
ok( $pi->get_termpos(), 2 );
ok( ++$pi, $ti->positionlist_end() );

ok( ++$ti ne $doc->termlist_end());
ok( $ti->get_termname(), 'baz' );
ok( $ti->get_wdf(), 2 );
$pi = $ti->positionlist_begin();
ok( $pi ne $ti->positionlist_end() );
ok( $pi->get_termpos(), 3 );
ok( ++$pi, $ti->positionlist_end() );

ok( ++$ti ne $doc->termlist_end() );
ok( $ti->get_termname(), 'foo' );
ok( $ti->get_wdf(), 2 );
$pi = $ti->positionlist_begin();
ok( $pi ne $ti->positionlist_end() );
ok( $pi->get_termpos(), 1 );
ok( ++$pi ne $ti->positionlist_end() );
ok( $pi->get_termpos(), 4 );
ok( ++$pi, $ti->positionlist_end() );

ok( ++$ti ne $doc->termlist_end() );
ok( $ti->get_termname(), 'zoo' );
ok( $ti->get_wdf(), 1 );
$pi = $ti->positionlist_begin();
ok( $pi, $ti->positionlist_end() );

ok( ++$ti eq $doc->termlist_end() );

my $db = Search::Xapian::WritableDatabase->new("testdb-spell", DB_CREATE_OR_OVERWRITE);
ok( $db );
my $indexer = Search::Xapian::TermGenerator->new();
$indexer->set_flags(Search::Xapian::FLAG_SPELLING);
$indexer->set_database($db);
my $document = Search::Xapian::Document->new();
$indexer->set_document($document);
$indexer->index_text('test hello');
$termgen->index_text('foo bar baz foo', 4);
$termgen->index_text_without_positions('baz zoo', 42);
ok( $db->add_document($document) );
undef $db;

1;
