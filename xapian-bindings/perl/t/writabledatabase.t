# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl test.pl'

#########################

use Test::More;
# Number of test cases to run - increase this if you add more testcases.
plan tests => 35;

use Search::Xapian qw(:standard);

my $db_dir = 'testdb-writabledatabase';

# Delete contents of database dir, if it exists.
if (opendir( DB_DIR, $db_dir )) {
  while( defined( my $file = readdir( DB_DIR ) ) ) {
    next if $file =~ /^\.+$/;
    unlink( "$db_dir/$file" ) or die "Could not delete '$db_dir/$file': $!";
  }
  closedir( DB_DIR );
}

my $write = Search::Xapian::WritableDatabase->new( $db_dir, Search::Xapian::DB_CREATE );

# Let's try to index something.
my $term = 'test';

for my $num (1..1000) {
  my $doc = Search::Xapian::Document->new();

  $doc->set_data( "$term $num" );

  $doc->add_posting( $term, 0 );
  $doc->add_posting( $num, 1 );

  $doc->add_value(0, $num);
  $write->add_document( $doc );
}

for my $num (qw(three four five)) {
  my $doc = Search::Xapian::Document->new();

  $doc->set_data( "$term $num" );

  $doc->add_posting( $term, 0 );
  $doc->add_posting( $num, 1 );

  $doc->add_value(0, $num);
  $write->add_document( $doc );
}
$write->flush();

my $doccount = $write->get_doccount();
is($doccount, 1003, "check number of documents in WritableDatabase");

# replace document by docid
my $repdoc = Search::Xapian::Document->new();
my $num = "six";
$term = "test";
my $docid = 500;
$repdoc->set_data( "$term $num" );
$repdoc->add_posting( $term, 0 );
$repdoc->add_posting( $num, 1 );
$repdoc->add_value(0, $num);

ok(!$write->term_exists($num), "check term exists");
is($write->get_document($docid)->get_data(), "$term $docid", "check document data");

$write->replace_document($docid, $repdoc);
$write->flush();

ok($write->term_exists($num), "check term exists");
is($write->get_document($docid)->get_data(), "$term $num", "check document data");

# replace document by term
$repdoc = Search::Xapian::Document->new();
$term = "test";
$num = "seven";
$repdoc->set_data( "$term $num" );
$repdoc->add_posting( $term, 0 );
$repdoc->add_posting( $num, 1 );
$repdoc->add_value(0, $num);
$repterm = "five";

ok(!$write->term_exists($num), "check term exists");
ok($write->term_exists($repterm), "check term exists");
is($write->get_termfreq($num), 0, "check term frequency");
is($write->get_termfreq($repterm), 1, "check term frequency");

$write->replace_document_by_term($repterm, $repdoc);
$write->flush();

ok($write->term_exists($num), "check term exists");
ok(!$write->term_exists($repterm), "check term exists");
is($write->get_termfreq($num), 1, "check term frequency");
is($write->get_termfreq($repterm), 0, "check term frequency");

# replace document by term, if term is new
$repdoc = Search::Xapian::Document->new();
$term = "test";
$num = "eight";
$repdoc->set_data( "$term $num" );
$repdoc->add_posting( $term, 0 );
$repdoc->add_posting( $num, 1 );
$repdoc->add_value(0, $num);

is($write->get_termfreq($term), $doccount, "check term frequency");
is($write->get_termfreq($num), 0, "check term frequency");

$write->replace_document_by_term($num, $repdoc);
$write->flush();

$doccount = $write->get_doccount();
is($doccount, 1004, "check doccount");
is($write->get_termfreq($term), $doccount, "check term frequency");
is($write->get_termfreq($num), 1, "check term frequency");

# replace document by term.
# all documents indexed with the term are replaced; the replacement uses the
# lowest docid if multiple documents are indexed by the term.
$repdoc = Search::Xapian::Document->new();
$term = "test";
$num = "nine";
$repdoc->set_data( "$term $num" );
$repdoc->add_posting( $term, 0 );
$repdoc->add_posting( $num, 1 );
$repdoc->add_value(0, $num);

$write->replace_document_by_term($term, $repdoc);
$write->flush();
my $doc = $write->get_document(1);

is($write->get_doccount(), 1, "check document count");
is($doc->get_data(), "$term $num", "check document data");

# add documents for following tests
for my $num (qw(one two three four five)) {
  my $doc = Search::Xapian::Document->new();

  $doc->set_data( "$term $num" );

  $doc->add_posting( $term, 0 );
  $doc->add_posting( $num, 1 );

  $doc->add_value(0, $num);
  $write->add_document( $doc );
}
$write->flush();

$doccount = $write->get_doccount();
is($doccount, 6, "check number of documents in WritableDatabase");

# delete document by docid
my $lastdocid = $write->get_lastdocid();
my $lastdocterm = $write->get_document($lastdocid)->get_value(0);
ok($write->term_exists($lastdocterm), "check term exists");

$write->delete_document($lastdocid);
$write->flush();

is($write->get_doccount(), $doccount - 1, "check number of documents in WritableDatabase");
ok(!$write->term_exists($lastdocterm), "check term exists");

# delete document by term
my $delterm = 'three';
ok($write->term_exists($delterm), 'check term exists before deleting a document');
is($write->get_termfreq($delterm), 1, 'check term frequency before deleting a document');

$write->delete_document_by_term($delterm);
$write->flush();

is($write->get_doccount(), $doccount - 2, 'check WritableDatabase after deleting a document');
ok(!$write->term_exists($delterm), 'check term exists after deleting a document');
is($write->get_termfreq($delterm), 0, 'check term frequency after deleting a document');

# delete documents by term
$delterm = 'test';
ok($write->term_exists($delterm), 'check term exists of documents which has term "test"');
is($write->get_termfreq($delterm), $doccount - 2, 'check term frequency of term "test"');

$write->delete_document_by_term($delterm);
$write->flush();

is($write->get_doccount(), 0, 'check WritableDatabase after deleting all documents');
ok(!$write->term_exists($delterm), 'check term exists after deleting all documents');
is($write->get_termfreq($delterm), 0, 'check term frequency after deleting all documents');

$write->close();
eval {
  # Should fail because the database has been closed.
  $write->add_document(Search::Xapian::Document->new());
};
ok( $@ );

1;
